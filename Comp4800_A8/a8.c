#define INITGUID
#define _USE_MATH_DEFINES
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <endpointvolume.h>
#include <stdio.h>
#include <math.h>

#define RATE 48000
#define LENGTH 12

int main() {
    CoInitialize(NULL);

    IMMDeviceEnumerator *devEnum = NULL;
    IMMDevice *outDevice = NULL;
    IAudioClient *client = NULL;
    IAudioRenderClient *renderer = NULL;

    // Get default device
    CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void**)&devEnum);
    devEnum->lpVtbl->GetDefaultAudioEndpoint(devEnum, eRender, eConsole, &outDevice);

    printf("Audio device ready\n");

    // Print device ID
    LPWSTR id = NULL;
    outDevice->lpVtbl->GetId(outDevice, &id);
    wprintf(L"ID: %ls\n", id);

    // Volume
    IAudioEndpointVolume *volInterface = NULL;
    outDevice->lpVtbl->Activate(outDevice, &IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, (void**)&volInterface);
    float volLevel;
    volInterface->lpVtbl->GetMasterVolumeLevelScalar(volInterface, &volLevel);
    printf("Volume Level: %.2f\n", volLevel);

    // Audio client
    outDevice->lpVtbl->Activate(outDevice, &IID_IAudioClient,CLSCTX_ALL, NULL, (void**)&client);
    WAVEFORMATEX *fmt;
    client->lpVtbl->GetMixFormat(client, &fmt);

    printf("Rate: %d Hz\n", fmt->nSamplesPerSec);

    client->lpVtbl->Initialize(client, AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, fmt, NULL);
    UINT32 bufferSize;
    client->lpVtbl->GetBufferSize(client, &bufferSize);

    client->lpVtbl->GetService(client, &IID_IAudioRenderClient,(void**)&renderer);

    client->lpVtbl->Start(client);

    double angle = 0.0;
    int framesLeft = RATE * LENGTH;
    int index = 0;

    while (framesLeft > 0) {
        UINT32 padding;
        client->lpVtbl->GetCurrentPadding(client, &padding);

        UINT32 available = bufferSize - padding;
        if (available == 0) continue;

        BYTE *raw;
        renderer->lpVtbl->GetBuffer(renderer, available, &raw);

        float *out = (float*)raw;

        for (UINT32 i = 0; i < available; i++, index++) {
            double t = (double)index / RATE;

            // dual tone (more distinct)
            double f1 = 300.0;
            double f2 = 600.0;

            // pulse effect
            double pulse = (sin(2 * M_PI * 1.5 * t) > 0) ? 1.0 : 0.3;

            float sample = (float)(pulse * (sin(angle) + 0.5 * sin(2 * angle)));

            angle += 2 * M_PI * f1 / RATE;
            if (angle > 2 * M_PI) angle -= 2 * M_PI;

            out[i * 2] = sample;
            out[i * 2 + 1] = sample;
        }

        renderer->lpVtbl->ReleaseBuffer(renderer, available, 0);
        framesLeft -= available;
    }

    client->lpVtbl->Stop(client);

    printf("Playback finished\n");

    return 0;
}