#ifndef CUDA_VOLUME_RENDERER_CU
#define CUDA_VOLUME_RENDERER_CU

//#include "Common.h"
#ifdef WIN32
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
#else

//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>
#endif
#include "../3rdparty/cudaHelper/helper_cuda.h"
#include "../3rdparty/cudaHelper/helper_math.h"
//#include "../3rdparty/cudaHelper/helper_cuda_gl.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
//#include <cuda_gl_interop.h>



typedef unsigned int  uint;
typedef unsigned char uchar;

cudaArray *d_volumeArray = 0;
cudaArray *d_transferFuncArray;

typedef unsigned char VolumeType;
//typedef unsigned short VolumeType;

texture<VolumeType, 3, cudaReadModeNormalizedFloat> tex;         // 3D texture
texture<float4, 1, cudaReadModeElementType>         transferTex; // 1D transfer function texture

typedef struct
{
    float4 m[3];
} float3x4;

__constant__ float3x4 c_invViewMatrix;

struct Ray
{
    float3 o;   // origin
    float3 d;   // direction
};


__device__
int intersectBox(Ray r, float3 boxmin, float3 boxmax, float *tnear, float *tfar)
{
    // compute intersection of ray with all six bbox planes
    float3 invR = make_float3(1.0f) / r.d;
    float3 tbot = invR * (boxmin - r.o);
    float3 ttop = invR * (boxmax - r.o);

    // re-order intersections to find smallest and largest on each axis
    float3 tmin = fminf(ttop, tbot);
    float3 tmax = fmaxf(ttop, tbot);

    // find the largest tmin and the smallest tmax
    float largest_tmin = fmaxf(fmaxf(tmin.x, tmin.y), fmaxf(tmin.x, tmin.z));
    float smallest_tmax = fminf(fminf(tmax.x, tmax.y), fminf(tmax.x, tmax.z));

    *tnear = largest_tmin;
    *tfar = smallest_tmax;

    return smallest_tmax > largest_tmin;
}

// transform vector by matrix (no translation)
__device__
float3 mul(const float3x4 &M, const float3 &v)
{
    float3 r;
    r.x = dot(v, make_float3(M.m[0]));
    r.y = dot(v, make_float3(M.m[1]));
    r.z = dot(v, make_float3(M.m[2]));
    return r;
}

// transform vector by matrix with translation
__device__
float4 mul(const float3x4 &M, const float4 &v)
{
    float4 r;
    r.x = dot(v, M.m[0]);
    r.y = dot(v, M.m[1]);
    r.z = dot(v, M.m[2]);
    r.w = 1.0f;
    return r;
}

__device__ uint rgbaFloatToInt(float4 rgba)
{
    rgba.x = __saturatef(rgba.x);   // clamp to [0.0, 1.0]
    rgba.y = __saturatef(rgba.y);
    rgba.z = __saturatef(rgba.z);
    rgba.w = __saturatef(rgba.w);
    return (uint(rgba.w*255)<<24) | (uint(rgba.z*255)<<16) | (uint(rgba.y*255)<<8) | uint(rgba.x*255);
}

__global__ void
d_render(uint *d_output, uint imageW, uint imageH, float3x4 invViewMatrix, float aspectRatio, float maxRaySteps, float rayStepSize)
{
    //const int maxSteps = 500;
    //const float tstep = 0.01f;
    const float opacityThreshold = 1.0f;
    const float3 boxMin = make_float3(-1.0f, -1.0f, -1.0f);
    const float3 boxMax = make_float3(1.0f, 1.0f, 1.0f);

    uint x = blockIdx.x*blockDim.x + threadIdx.x;
    uint y = blockIdx.y*blockDim.y + threadIdx.y;

    if ((x >= imageW) || (y >= imageH)) return;

    float u = ((x / (float) imageW)*2.0f-1.0f) * aspectRatio;
    float v = (y / (float) imageH)*2.0f-1.0f;

    // calculate eye ray in model space
    Ray eyeRay;
    eyeRay.o = make_float3(mul(invViewMatrix, make_float4(0.0f, 0.0f, 0.0f, 1.0f)));
    eyeRay.d = normalize(make_float3(u, v, -2.0f));
    eyeRay.d = mul(invViewMatrix, eyeRay.d);

    // find intersection with box
    float tnear, tfar;
    int hit = intersectBox(eyeRay, boxMin, boxMax, &tnear, &tfar);

    if (!hit) return;

    if (tnear < 0.0f) tnear = 0.0f;     // clamp to near plane

    // march along ray from front to back, accumulating color
    float4 sum = make_float4(0.0f);
    float t = tnear;
    float3 pos = eyeRay.o + eyeRay.d*tnear;
    float3 step = eyeRay.d*rayStepSize;

    for (int i=0; i<maxRaySteps; i++)
    {
        // read from 3D texture
        // remap position to [0, 1] coordinates
		//float xx =pos.x*0.5f+0.5f;
		//float yy =pos.y*0.5f+0.5f;
		//float zz =pos.z*0.5f+0.5f;
		//float sample = 0;
        float sample = tex3D(tex, pos.x*0.5f+0.5f, pos.y*0.5f+0.5f, pos.z*0.5f+0.5f);
        //sample *= 64.0f;    // scale for 10-bit data

        // lookup in transfer function texture
        //float4 col = tex1D(transferTex, (sample-transferOffset)*transferScale);
        float4 col = tex1D(transferTex, sample);
		//float4 col = make_float4(0);
       // col.w *= 0.05f;

        // "under" operator for back-to-front blending
        //sum = lerp(sum, col, col.w);

        // pre-multiply alpha
        col.x *= col.w;
        col.y *= col.w;
        col.z *= col.w;
        // "over" operator for front-to-back blending
        sum = sum + col*(1.0f - sum.w);

        // exit early if opaque
        if (t > tfar || sum.w > opacityThreshold)
		{
			//sum += make_float4(1.0f) * (1.0f - sum.w);
            break;
		}

        t += rayStepSize;
        pos += step;
    }

   // sum *= brightness;

    // write output color
    d_output[y*imageW + x] = rgbaFloatToInt(sum);
}



extern "C"
void initCuda()
{
//	    // Otherwise pick the device with highest Gflops/s
//    int devID = gpuGetMaxGflopsDeviceId();
//    cudaGLSetGLDevice(devID);
}
extern "C" void exitCuda()
{
    checkCudaErrorsLog(cudaFreeArray(d_volumeArray));
    checkCudaErrorsLog(cudaFreeArray(d_transferFuncArray));
}

extern "C" void writeVolume(void *volume, cudaExtent volumeSize)
{
	if(d_volumeArray)
	{
		    checkCudaErrorsLog(cudaFreeArray(d_volumeArray));
	}
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<VolumeType>();
    checkCudaErrorsLog(cudaMalloc3DArray(&d_volumeArray, &channelDesc, volumeSize));

    // copy data to 3D array
    cudaMemcpy3DParms copyParams = {0};
    copyParams.srcPtr   = make_cudaPitchedPtr(volume, volumeSize.width*sizeof(VolumeType), volumeSize.width, volumeSize.height);
    copyParams.dstArray = d_volumeArray;
    copyParams.extent   = volumeSize;
    copyParams.kind     = cudaMemcpyHostToDevice;
    checkCudaErrorsLog(cudaMemcpy3D(&copyParams));

    // set texture parameters
    tex.normalized = true;                      // access with normalized texture coordinates
    tex.filterMode = cudaFilterModeLinear;      // linear interpolation
    tex.addressMode[0] = cudaAddressModeClamp;  // clamp texture coordinates
    tex.addressMode[1] = cudaAddressModeClamp;

    // bind array to 3D texture
    checkCudaErrorsLog(cudaBindTextureToArray(tex, d_volumeArray, channelDesc));
}

extern "C" void writeTransferFunction(float *transferFunction, int transferFunctionSize)
{
	if(d_transferFuncArray)
	{
		checkCudaErrorsLog(cudaFreeArray(d_transferFuncArray));
	}
	cudaChannelFormatDesc channelDesc2 = cudaCreateChannelDesc<float4>();
    cudaArray *d_transferFuncArray;
    checkCudaErrorsLog(cudaMallocArray(&d_transferFuncArray, &channelDesc2, transferFunctionSize, 1));
    checkCudaErrorsLog(cudaMemcpyToArray(d_transferFuncArray, 0, 0, transferFunction, transferFunctionSize * 4 * sizeof(float), cudaMemcpyHostToDevice));
    //checkCudaErrorsLog(cudaMallocArray(&d_transferFuncArray, &channelDesc2, sizeof(transferFunc)/sizeof(float4), 1));
    //checkCudaErrorsLog(cudaMemcpyToArray(d_transferFuncArray, 0, 0, transferFunc, sizeof(transferFunc), cudaMemcpyHostToDevice));


    transferTex.filterMode = cudaFilterModeLinear;
    transferTex.normalized = true;    // access with normalized texture coordinates
    transferTex.addressMode[0] = cudaAddressModeClamp;   // wrap texture coordinates

    // Bind the array to the texture
    checkCudaErrorsLog(cudaBindTextureToArray(transferTex, d_transferFuncArray, channelDesc2));

}

extern "C" void initCudaVolume(void *volume, cudaExtent volumeSize, float *transferFunction, int transferFunctionSize)
{
	    // create 3D array
	writeVolume(volume, volumeSize);
	writeTransferFunction(transferFunction, transferFunctionSize);

    //float4 transferFunc[] =
    //{
    //    {  0.0, 0.0, 0.0, 0.0, },
    //    {  1.0, 0.0, 0.0, 1.0, },
    //    {  1.0, 0.5, 0.0, 1.0, },
    //    {  1.0, 1.0, 0.0, 1.0, },
    //    {  0.0, 1.0, 0.0, 1.0, },
    //    {  0.0, 1.0, 1.0, 1.0, },
    //    {  0.0, 0.0, 1.0, 1.0, },
    //    {  1.0, 0.0, 1.0, 1.0, },
    //    {  0.0, 0.0, 0.0, 0.0, },
    //};
	//CT-Knee transfer function
    //float4 transferFunc[] =
    //{
    //    {  0.0, 0.0, 0.0, 0.0, },
    //    {  1.0, 0.0, 0.0, 8.0f/100.0f, },
    //    {  1.0, 95.0f/255.0f, 0.0, 0.0, },
    //    {  1.0, 191.0f/255.0f, 0.0, 0.0, },
    //    {  223.0f/255.0f, 1.0f, 0.0, 1.0f/100.0f, },
    //    {  127.0f/255.0f, 1.0f, 0.0, 12.0f/100.0f, },
    //    {  31.0f/255.0f, 1.0f, 0.0, 2.0f/100.0f, },
    //    {  0.0, 1.0f, 63.0f/255.0f, 5.0/100.0f, },
    //    {  0.0, 1.0f, 159.0f/255.0f, 24.0f/100.0f, },
    //    {  0.0, 1.0f, 1.0f, 18.0f/100.0f, },
    //    {  0.0, 159.0f/255.0f, 1.0f, 24.0f/100.0f, },
    //    {  0.0, 63.0f/255.0f, 1.0f, 30.0f/100.0f, },
    //    {  31.0f/255.0f, 0.0, 1.0f, 34.0f/100.0f, },
    //    {  127.0f/255.0f, 0.0, 1.0f, 24.0f/100.0f, },
    //    {  223.0f/255.0f, 0.0, 1.0f, 21.0f/100.0f, },
    //    {  1.0f, 0.0, 191.0f/255.0f, 53.0f/100.0f, },
    //    {  1.0f, 0.0, 95.0f/255.0f, 61.0f/100.0f, },
    //    {  1.0f, 1.0f, 1.0f, 0.0f/100.0f, },
    //};
}


extern "C" void render_kernel(dim3 gridSize, dim3 blockSize, uint *d_output, uint imageW, uint imageH, float *invViewMatrix, float aspectRatio, float maxRaySteps, float rayStepSize)
{
	float3x4 cudaInvViewMatrix;
	cudaInvViewMatrix.m[0].x = invViewMatrix[0];
	cudaInvViewMatrix.m[0].y = invViewMatrix[1];
	cudaInvViewMatrix.m[0].z = invViewMatrix[2];
	cudaInvViewMatrix.m[0].w = invViewMatrix[3];

	cudaInvViewMatrix.m[1].x = invViewMatrix[4];
	cudaInvViewMatrix.m[1].y = invViewMatrix[5];
	cudaInvViewMatrix.m[1].z = invViewMatrix[6];
	cudaInvViewMatrix.m[1].w = invViewMatrix[7];

	cudaInvViewMatrix.m[2].x = invViewMatrix[8];
	cudaInvViewMatrix.m[2].y = invViewMatrix[9];
	cudaInvViewMatrix.m[2].z = invViewMatrix[10];
	cudaInvViewMatrix.m[2].w = invViewMatrix[11];
    d_render<<<gridSize, blockSize>>>(d_output, imageW, imageH, cudaInvViewMatrix, aspectRatio, maxRaySteps, rayStepSize);
}


extern "C" void copyInvViewMatrix(float *invViewMatrix, size_t sizeofMatrix)
{
    checkCudaErrorsLog(cudaMemcpyToSymbol(c_invViewMatrix, invViewMatrix, sizeofMatrix));
}




#endif // !CUDA_VOLUME_RENDERER_CU
