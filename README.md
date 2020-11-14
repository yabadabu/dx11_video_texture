### Sample project to play .mp4 files with DX11

This is a sample I would have like to have found long time ago, but I was unable to find a simple sample to play mp4 videos using DX11.

The required code is inside video_texture.cpp. 

I'm using Media Foundation API to decode the mp4 file. The resulting frames come in IYUV format. If I force the output of the MF to be RGBA the performance drops dramatically. So instead we upload the output buffer to two dynamic directX textures, one to hold the Y and another to hold the UV data. Then, a pixel shader is used to do the conversion from YUV to RGB. 

I have tried to use the NV12 format without success because the layout of the YUV data in the NV12 does not match the layout in the IYUV format. 
Instead, the directX textures I use have the format DXGI_FORMAT_R8_UNORM, and in the pixel shader I sample the Y, U and V information playing with the texture coords.

Just for reference, a frame with size 8x4 returned by the MF library in IYUV has the following layout:

YYYY YYYY
YYYY YYYY
YYYY YYYY
YYYY YYYY
UUUU
UUUU
VVVV
VVVV

So, we split the data in two textures. Luminance 8x4, and UV 4x4

YYYY YYYY
YYYY YYYY
YYYY YYYY
YYYY YYYY

UUUU
UUUU
VVVV
VVVV

Accessing the Y texture requires no modification. For U divide the y component of the texture coordinate by 2. Finally, for the V divide and add 0.5 to match the start of the V data in the texture. Then use your favourite YUV 2 RGB formula to obtain the final RGB color.

In my machine the full operation in the CPU takes less than 0.5ms for a full 1080 video.

Final note, when playing videos with vertical resolution not multiple of 16, the MF Api properly returns the correct video size, like 1080, but then in the first frame it reports a media type change event, to report the a 1088 vertical resolution. That's why I wait until the first frame is decoded to create the DX textures.

No synchronization primitive is performed beyond checking if a new frame should be decoded as it's now superfast and the MF provide absolute times for each frame.

No audio output or playing is provided, but MF can provide this data also.

I have tried to put everything in a single texture (xres) x (yres x 3 / 2), but correctly accessing the U and V information was why more difficult.

## Credits

Most of the MF code is taken directly from the Microsoft sample mediafoundationsamples\MFMP4ToYUVWithoutMFT
The sample video has been downloaded from www.videvo.net

