/*
 * Copyright 2020-2021 Leo McCormack
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file test__utilities_module.c
 * @brief Unit tests for the SAF utilities module
 * @author Leo McCormack
 * @date 27.04.2020
 * @license ISC
 */

#include "saf_test.h"

void test__cart2sph(void){
    const float acceptedTolerance = 0.00001f;
    float cordCar [100][3];
    float cordSph [100][3];
    float cordCarTest [100][3];

    /* Generate some random Cartesian coordinates */
    rand_m1_1((float*) cordCar, 100*3);

    /* rad */
    cart2sph((float*) cordCar, 100, SAF_FALSE, (float*) cordSph);
    sph2cart((float*) cordSph, 100, SAF_FALSE, (float*) cordCarTest);
    for (int i=0; i<100; i++)
        for (int j=0; j<3; j++)
            TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, cordCar[i][j], cordCarTest[i][j]);

    /*deg */
    cart2sph((float*) cordCar, 100, SAF_TRUE, (float*) cordSph);
    sph2cart((float*) cordSph, 100, SAF_TRUE, (float*) cordCarTest);
    for (int i=0; i<100; i++)
        for (int j=0; j<3; j++)
            TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, cordCar[i][j], cordCarTest[i][j]);
}

void test__delaunaynd(void){
    int nMesh;
    int* mesh;

    /* Not really a unit test... You have to copy the mesh indices into e.g. Matlab, plot, and see... */

    /* 2D 3 points */
    float three_xy[3][2] =
      { {7.0f,7.0f},{2.0f,7.0f},{2.0f,1.0f} };
    mesh = NULL;
    delaunaynd((float*)three_xy, 3, 2/*nDims*/, &mesh, &nMesh);
    free(mesh);

    /* 2D 4 points */
    float four_xy[4][2] =
      { {7.0f,7.0f},{2.0f,7.0f},{2.0f,1.0f},{7.0f,1.0f} };
    mesh = NULL;
    delaunaynd((float*)four_xy, 4, 2/*nDims*/, &mesh, &nMesh);
    free(mesh);

    /* 2D Square */
    float square_xy[26][2] =
      { {-1.0f,-1.0f},{-1.0f,-0.5f},{-1.0f,0.0f},{-1.0f,0.5f},{-1.0f,1.0f},{-0.5f,-1.0f},{-0.5f,-0.5f},{-0.5f,0.0f},{-0.5f,0.5f},
        {-0.5f,1.0f},{0.0f,-1.0f},{0.0f,-0.5f},{0.0f,0.0f},{0.0f,0.5f},{0.0f,1.0f},{0.5f,-1.0f},
        {0.5f,-0.5f},{0.5f,0.0f},{0.5f,0.5f},{0.5f,1.0f},{1.0f,-1.0f},{1.0f,-0.5f},
        {1.0f,0.0f},{1.0f,0.5f},{1.0f,1.0f},{0.0f,0.0f} };
    mesh = NULL;
    delaunaynd((float*)square_xy, 26, 2/*nDims*/, &mesh, &nMesh);
    free(mesh);

    /* 3D Cube */
    float cube_xyz[8][3] =
      { {-1.0f,-1.0f,-1.0f},{-1.0f,1.0f,-1.0f},{1.0f,-1.0f,-1.0f},{1.0f,1.0f,-1.0f},
        {-1.0f,-1.0f,1.0f}, {-1.0f,1.0f,1.0f}, {1.0f,-1.0f,1.0f}, {1.0f,1.0f,1.0f} };
    mesh = NULL;
    delaunaynd((float*)cube_xyz, 8, 3/*nDims*/, &mesh, &nMesh);
    free(mesh);

    /* 3D Cube with a point in the centre */
    float cube_xyz2[9][3] =
      { {-1.0f,-1.0f,-1.0f},{-1.0f,1.0f,-1.0f},{1.0f,-1.0f,-1.0f},{1.0f,1.0f,-1.0f},
        {-1.0f,-1.0f,1.0f}, {-1.0f,1.0f,1.0f}, {1.0f,-1.0f,1.0f}, {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f} };
    mesh = NULL;
    delaunaynd((float*)cube_xyz2, 9, 3/*nDims*/, &mesh, &nMesh);
    free(mesh);
}

void test__quaternion(void){
    int i, j;
    float norm;
    float rot[3][3], rot2[3][3], residual[9], ypr[3], test_ypr[3];
    quaternion_data Q, Q1, Q2;

    for(i=0; i<1000; i++){
        /* Randomise the quaternion values */
        rand_m1_1(Q.Q, 4);

        /* Normalise to make it valid */
        norm = L2_norm(Q.Q, 4);
        Q.w /= norm;
        Q.x /= norm;
        Q.y /= norm;
        Q.z /= norm;
        /* Q.w = 0; Q.x = 0.0000563298236; Q.y = 0.947490811; Q.z = -0.319783032; // Problem case! */

        /* Convert to rotation matrix, then back, then to rotation matrix again */
        quaternion2rotationMatrix(&Q, rot);
        rotationMatrix2quaternion(rot, &Q1);
        quaternion2rotationMatrix(&Q1, rot2);

        /* Ensure that the difference between them is 0 */
        utility_svvsub((float*)rot, (float*)rot2, 9, residual);
        for(j=0; j<9; j++)
            TEST_ASSERT_TRUE(fabsf(residual[j])<1e-3f);

        /* Testing that quaternion2euler() and euler2Quaternion() are invertable */
        quaternion2euler(&Q1, 1, EULER_ROTATION_YAW_PITCH_ROLL, &ypr[0], &ypr[1], &ypr[2]);
        euler2Quaternion(ypr[0], ypr[1], ypr[2], 1, EULER_ROTATION_YAW_PITCH_ROLL, &Q2);
        quaternion2euler(&Q2, 1, EULER_ROTATION_YAW_PITCH_ROLL, &test_ypr[0], &test_ypr[1], &test_ypr[2]);
        for(j=0; j<3; j++)
            TEST_ASSERT_TRUE(fabsf(test_ypr[j]-ypr[j])<1e-2f);
    }
}

void test__saf_stft_50pc_overlap(void){
    int frame, winsize, hopsize, nFrames, ch, i, nBands, nTimesSlots, band;
    void* hSTFT;
    float** insig, **outsig, **inframe, **outframe;
    float_complex*** inspec, ***outspec;

    /* prep */
    const float acceptedTolerance = 0.000001f;
    const int fs = 48000;
    const int signalLength = 1*fs;
    const int framesize = 512;
    const int nCHin = 62;
    const int nCHout = 64;
    insig = (float**)malloc2d(nCHin,signalLength,sizeof(float)); /* One second long */
    outsig = (float**)malloc2d(nCHout,signalLength,sizeof(float));
    inframe = (float**)malloc2d(nCHin,framesize,sizeof(float));
    outframe = (float**)malloc2d(nCHout,framesize,sizeof(float));
    rand_m1_1(FLATTEN2D(insig), nCHin*signalLength); /* populate with random numbers */

    /* Set-up STFT for 50% overlapping */
    winsize = 128;
    hopsize = winsize/2;
    nBands = winsize+1;
    nTimesSlots = framesize/hopsize;
    inspec = (float_complex***)malloc3d(nBands, nCHin, nTimesSlots, sizeof(float_complex));
    outspec = (float_complex***)malloc3d(nBands, nCHout, nTimesSlots, sizeof(float_complex));
    saf_stft_create(&hSTFT, winsize, hopsize, nCHin, nCHout, SAF_STFT_BANDS_CH_TIME);
    saf_stft_channelChange(hSTFT, 123, 7);        /* messing about */
    saf_stft_flushBuffers(hSTFT);                 /* messing about */
    saf_stft_channelChange(hSTFT, nCHin, nCHout); /* change back */

    /* Pass insig through STFT, block-wise processing */
    nFrames = (int)((float)signalLength/(float)framesize);
    for(frame = 0; frame<nFrames; frame++){
        /* Forward */
        for(ch=0; ch<nCHin; ch++)
            memcpy(inframe[ch], &insig[ch][frame*framesize], framesize*sizeof(float));
        saf_stft_forward(hSTFT, inframe, framesize, inspec);

        /* Copy first channel of inspec to all outspec channels */
        for(band=0; band<nBands; band++)
            for(ch=0; ch<nCHout; ch++)
                memcpy(outspec[band][ch], inspec[band][0], nTimesSlots*sizeof(float_complex));

        /* Backward */
        saf_stft_backward(hSTFT, outspec, framesize, outframe);
        for(ch=0; ch<nCHout; ch++)
            memcpy(&outsig[ch][frame*framesize], outframe[ch], framesize*sizeof(float));
    }

    /* Check that input==output (given some numerical precision) */
    for(i=0; i<signalLength-framesize; i++)
        TEST_ASSERT_TRUE( fabsf(insig[0][i] - outsig[0][i+hopsize]) <= acceptedTolerance );

    /* Clean-up */
    saf_stft_destroy(&hSTFT);
    free(insig);
    free(outsig);
    free(inframe);
    free(outframe);
    free(inspec);
    free(outspec);
}

void test__saf_stft_LTI(void){
    int frame, winsize, hopsize, nFrames, ch, i, nBands, nTimesSlots, band;
    void* hSTFT;
    float** insig, **outsig, **inframe, **outframe;
    float_complex*** inspec, ***outspec;

    /* prep */
    const float acceptedTolerance = 0.000001f;
    const int fs = 48000;
    const int framesize = 128;
    const int nCHin = 62;
    const int nCHout = 64;
    insig = (float**)malloc2d(nCHin,fs,sizeof(float)); /* One second long */
    outsig = (float**)malloc2d(nCHout,fs,sizeof(float));
    inframe = (float**)malloc2d(nCHin,framesize,sizeof(float));
    outframe = (float**)malloc2d(nCHout,framesize,sizeof(float));
    rand_m1_1(FLATTEN2D(insig), nCHin*fs); /* populate with random numbers */

    /* Set-up STFT suitable for LTI filtering applications */
    winsize = hopsize = 128;
    nBands = winsize+1;
    nTimesSlots = framesize/hopsize;
    inspec = (float_complex***)malloc3d(nBands, nCHin, nTimesSlots, sizeof(float_complex));
    outspec = (float_complex***)malloc3d(nBands, nCHout, nTimesSlots, sizeof(float_complex));
    saf_stft_create(&hSTFT, winsize, hopsize, nCHin, nCHout, SAF_STFT_BANDS_CH_TIME);

    /* Pass insig through STFT, block-wise processing */
    nFrames = (int)((float)fs/(float)framesize);
    for(frame = 0; frame<nFrames; frame++){
        /* Forward */
        for(ch=0; ch<nCHin; ch++)
            memcpy(inframe[ch], &insig[ch][frame*framesize], framesize*sizeof(float));
        saf_stft_forward(hSTFT, inframe, framesize, inspec);

        /* Copy first channel of inspec to all outspec channels */
        for(band=0; band<nBands; band++)
            for(ch=0; ch<nCHout; ch++)
                memcpy(outspec[band][ch], inspec[band][0], nTimesSlots*sizeof(float_complex));

        /* Backward */
        saf_stft_backward(hSTFT, outspec, framesize, outframe);
        for(ch=0; ch<nCHout; ch++)
            memcpy(&outsig[ch][frame*framesize], outframe[ch], framesize*sizeof(float));
    }

    /* Check that input==output (given some numerical precision) */
    for(i=0; i<fs-framesize; i++)
        TEST_ASSERT_TRUE( fabsf(insig[0][i] - outsig[63][i]) <= acceptedTolerance );

    /* Clean-up */
    saf_stft_destroy(&hSTFT);
    free(insig);
    free(outsig);
    free(inframe);
    free(outframe);
    free(inspec);
    free(outspec);
}

void test__saf_matrixConv(void){
    int i, frame;
    float** inputTD, **outputTD, **inputFrameTD, **outputFrameTD;
    float*** filters;
    void* hMatrixConv;

    /* config */
    const int signalLength = 48000;
    const int hostBlockSize = 2048;
    const int filterLength = 512;
    const int nInputs = 32;
    const int nOutputs = 40;

    /* prep */
    inputTD = (float**)malloc2d(nInputs, signalLength, sizeof(float));
    outputTD = (float**)malloc2d(nOutputs, signalLength, sizeof(float));
    inputFrameTD = (float**)malloc2d(nInputs, hostBlockSize, sizeof(float));
    outputFrameTD = (float**)calloc2d(nOutputs, hostBlockSize, sizeof(float));
    filters = (float***)malloc3d(nOutputs, nInputs, filterLength, sizeof(float));
    rand_m1_1(FLATTEN3D(filters), nOutputs*nInputs*filterLength);
    rand_m1_1(FLATTEN2D(inputTD), nInputs*signalLength);
    saf_matrixConv_create(&hMatrixConv, hostBlockSize, FLATTEN3D(filters), filterLength,
                          nInputs, nOutputs, SAF_TRUE);

    /* Apply */
    for(frame = 0; frame<(int)signalLength/hostBlockSize; frame++){
        for(i = 0; i<nInputs; i++)
            memcpy(inputFrameTD[i], &inputTD[i][frame*hostBlockSize], hostBlockSize*sizeof(float));

        saf_matrixConv_apply(hMatrixConv, FLATTEN2D(inputFrameTD), FLATTEN2D(outputFrameTD));

        for(i = 0; i<nOutputs; i++)
            memcpy(&outputTD[i][frame*hostBlockSize], outputFrameTD[i], hostBlockSize*sizeof(float));
    }

    /* Clean-up */
    free(inputTD);
    free(outputTD);
    free(inputFrameTD);
    free(outputFrameTD);
    free(filters);
    saf_matrixConv_destroy(&hMatrixConv);
}

void test__saf_rfft(void){
    int i, j, N;
    float* x_td, *test;
    float_complex* x_fd;
    void *hFFT;

    /* Config */
    const float acceptedTolerance = 0.00001f;
    const int fftSizesToTest[24] =
        {16,256,512,1024,2048,4096,8192,16384,32768,65536,1048576,     /*     2^x */
         80,160,320,640,1280,240,480,960,1920,3840,7680,15360,30720 }; /* non-2^x, (but still supported by vDSP) */

    /* Loop over the different FFT sizes */
    for (i=0; i<24; i++){
        N = fftSizesToTest[i];

        /* prep */
        x_td = malloc1d(N*sizeof(float));
        test = malloc1d(N*sizeof(float));
        x_fd = malloc1d((N/2+1)*sizeof(float_complex));
        rand_m1_1(x_td, N); /* populate with random numbers */
        saf_rfft_create(&hFFT, N);

        /* forward and backward transform */
        saf_rfft_forward(hFFT, x_td, x_fd);
        saf_rfft_backward(hFFT, x_fd, test);

        /* Check that, x_td==test */
        for(j=0; j<N; j++)
            TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, x_td[j], test[j]);

        /* clean-up */
        saf_rfft_destroy(&hFFT);
        free(x_fd);
        free(x_td);
        free(test);
    }
}

void test__saf_fft(void){
    int i, j, N;
    float_complex* x_td, *test;
    float_complex* x_fd;
    void *hFFT;

    /* Config */
    const float acceptedTolerance = 0.00001f;
    const int fftSizesToTest[24] =
        {16,256,512,1024,2048,4096,8192,16384,32768,65536,1048576,     /*     2^x */
         80,160,320,640,1280,240,480,960,1920,3840,7680,15360,30720 }; /* non-2^x, (but still supported by vDSP) */

    /* Loop over the different FFT sizes */
    for (i=0; i<24; i++){
        N = fftSizesToTest[i];

        /* prep */
        x_td = malloc1d(N*sizeof(float_complex));
        test = malloc1d(N*sizeof(float_complex));
        x_fd = malloc1d(N*sizeof(float_complex));
        rand_m1_1((float*)x_td, N*2); /* populate with random numbers */
        saf_fft_create(&hFFT, N);

        /* forward and backward transform */
        saf_fft_forward(hFFT, x_td, x_fd);
        saf_fft_backward(hFFT, x_fd, test);

        /* Check that, x_td==test */
        for(j=0; j<N; j++){
            TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, crealf(x_td[j]), crealf(test[j]));
            TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, cimagf(x_td[j]), cimagf(test[j]));
        }

        /* clean-up */
        saf_fft_destroy(&hFFT);
        free(x_fd);
        free(x_td);
        free(test);
    }
}

void test__qmf(void){
    int frame, nFrames, ch, i, nBands, procDelay, band, nHops;
    void* hQMF;
    float* freqVector;
    float** insig, **outsig, **inframe, **outframe;
    float_complex*** inspec, ***outspec;

    /* prep */
    const float acceptedTolerance = 0.01f;
    const int fs = 48000;
    const int signalLength = 1*fs;
    const int framesize = 512;
    const int hopsize = 128;
    const int nCHin = 60;
    const int hybridMode = 1;
    const int nCHout = 64;
    insig = (float**)malloc2d(nCHin,signalLength,sizeof(float)); /* One second long */
    outsig = (float**)malloc2d(nCHout,signalLength,sizeof(float));
    inframe = (float**)malloc2d(nCHin,framesize,sizeof(float));
    outframe = (float**)malloc2d(nCHout,framesize,sizeof(float));
    rand_m1_1(FLATTEN2D(insig), nCHin*signalLength); /* populate with random numbers */

    /* Set-up */
    nHops = framesize/hopsize;
    qmf_create(&hQMF, nCHin, nCHout, hopsize, hybridMode, QMF_BANDS_CH_TIME);
    procDelay = qmf_getProcDelay(hQMF);
    nBands = qmf_getNBands(hQMF);
    freqVector = malloc1d(nBands*sizeof(float));
    qmf_getCentreFreqs(hQMF, (float)fs, nBands, freqVector);
    inspec = (float_complex***)malloc3d(nBands, nCHin, nHops, sizeof(float_complex));
    outspec = (float_complex***)malloc3d(nBands, nCHout, nHops, sizeof(float_complex));

    /* Pass insig through the QMF filterbank, block-wise processing */
    nFrames = (int)((float)signalLength/(float)framesize);
    for(frame = 0; frame<nFrames; frame++){
        /* QMF Analysis */
        for(ch=0; ch<nCHin; ch++)
            memcpy(inframe[ch], &insig[ch][frame*framesize], framesize*sizeof(float));
        qmf_analysis(hQMF, inframe, framesize, inspec);

        /* Copy first channel of inspec to all outspec channels */
        for(band=0; band<nBands; band++)
            for(ch=0; ch<nCHout; ch++)
                memcpy(outspec[band][ch], inspec[band][0], nHops*sizeof(float_complex));

        /* QMF Synthesis */
        qmf_synthesis(hQMF, outspec, framesize, outframe);
        for(ch=0; ch<nCHout; ch++)
            memcpy(&outsig[ch][frame*framesize], outframe[ch], framesize*sizeof(float));
    }

    /* Check that input==output (given some numerical precision) - channel 0 */
    for(i=0; i<signalLength-procDelay-framesize; i++)
        TEST_ASSERT_TRUE( fabsf(insig[0][i] - outsig[0][i+procDelay]) <= acceptedTolerance );

    /* Clean-up */
    qmf_destroy(&hQMF);
    free(insig);
    free(outsig);
    free(inframe);
    free(outframe);
    free(inspec);
    free(outspec);
    free(freqVector);
}

void test__smb_pitchShifter(void){
    float* inputData, *outputData;
    void* hPS, *hFFT;
    float frequency;
    int i, smbLatency, ind;

    /* Config */
    const int sampleRate = 48000;
    const int FFTsize = 8192;
    const int osfactor = 4;
    const int nSamples = 8*FFTsize;

    /* prep */
    smb_pitchShift_create(&hPS, 1, FFTsize, osfactor, (float)sampleRate);
    inputData = malloc1d(nSamples*sizeof(float));
    outputData = calloc1d(nSamples,sizeof(float));
    frequency = (float)sampleRate/8.0f;
    for(i=0; i<nSamples; i++) /* sine tone at quarter Nyquist: */
        inputData[i] = sinf(2.0f * M_PI * (float)i * frequency/(float)sampleRate);
    smbLatency = FFTsize - (FFTsize/osfactor);

    /* Pitch shift down one octave */
    smb_pitchShift_apply(hPS, 0.5, nSamples, inputData, outputData);

    /* Take FFT, the bin with the highest energy should correspond to 1/8 Nyquist */
    float_complex* out_fft; // [nSamples / 2 + 1];
    out_fft = malloc1d((nSamples / 2 + 1) * sizeof(float_complex));
    saf_rfft_create(&hFFT, nSamples);
    saf_rfft_forward(hFFT, outputData, out_fft);
    utility_cimaxv(out_fft, nSamples/2+1, &ind);
    TEST_ASSERT_TRUE(ind == nSamples/16);

    /* clean-up */
    smb_pitchShift_destroy(&hPS);
    saf_rfft_destroy(&hFFT);
    free(inputData);
    free(outputData);
    free(out_fft);
}

void test__sortf(void){
    float* values;
    int* sortedIdx;
    int i;

    /* Config */
    const int numValues = 10000;

    /* Prep */
    sortedIdx = malloc1d(numValues*sizeof(int));
    values = malloc1d(numValues*sizeof(float));
    rand_m1_1(values, numValues); /* populate with random numbers */

    /* Sort in accending order */
    sortf(values, NULL, sortedIdx, numValues, 0);

    /* Check that the next value is either the same or greater than current value */
    for(i=0; i<numValues-1; i++)
        TEST_ASSERT_TRUE(values[sortedIdx[i]]<=values[sortedIdx[i+1]]);

    /* Sort in decending order */
    sortf(values, NULL, sortedIdx, numValues, 1);

    /* Check that the next value is either the same or less than current value */
    for(i=0; i<numValues-1; i++)
        TEST_ASSERT_TRUE(values[sortedIdx[i]]>=values[sortedIdx[i+1]]);

    /* clean-up */
    free(values);
    free(sortedIdx);
}

void test__sortz(void){
    int i;
    const int N = 36;
    double_complex vals[36] ={
        cmplx(1.0, 1.0), cmplx(7.0, 1.0),  cmplx(10.0, 5.0),
        cmplx(12.0, 4.0), cmplx(4.0, 4.0), cmplx(8.0, 0.0),
        cmplx(10.0, -1.0), cmplx(7.0, 5.0), cmplx(7.0, 2.0),
        cmplx(5.0, 1.0), cmplx(4.0, -1.0), cmplx(32.0, 3.0),
        cmplx(32.0, 32.5), cmplx(25.0, 0.0), cmplx(2.0, -2.0),
        cmplx(7.0, -2.0), cmplx(1.0, -1.0), cmplx(12.0, -1.0),
        cmplx(2.0, -1.0), cmplx(4.0, 2.0), cmplx(10.0, 6.0),
        cmplx(5.0, 2.0), cmplx(32.0, 1.5), cmplx(7.0, -10.0),
        cmplx(1.0, -1.5), cmplx(4.0, 25.0), cmplx(3.0, 2.0),
        cmplx(1.0, 4.5), cmplx(10.0, 5.0), cmplx(10.0, 2.0),
        cmplx(10.0, -3.5), cmplx(30.0, -10.0), cmplx(7.0, -12.0),
        cmplx(1.0, -13.5), cmplx(12.0, -12.0), cmplx(32.0, 23.0)
    };
    double_complex sorted_vals[36];

    /* Sort assending order */
    sortz(vals, sorted_vals, N, 0);

    /* Check that the next real(value) is either the same or greater than current real(value) */
    for(i=0; i<N-1; i++)
        TEST_ASSERT_TRUE(creal(sorted_vals[i])<=creal(sorted_vals[i+1]));

    /* Check that if the next real(value) is the same as the current real(value), then
     * the next imag(value) is greater that the current image(value)*/
    for(i=0; i<N-1; i++)
        if(fabs(creal(sorted_vals[i])-creal(sorted_vals[i+1])) < 0.00001 )
            TEST_ASSERT_TRUE(cimag(sorted_vals[i])<=cimag(sorted_vals[i+1]));

    /* Sort decending order */
    sortz(vals, sorted_vals, N, 1);

    /* Check that the next real(value) is either the same or smaller than current real(value) */
    for(i=0; i<N-1; i++)
        TEST_ASSERT_TRUE(creal(sorted_vals[i])>=creal(sorted_vals[i+1]));

    /* Check that if the next real(value) is the same as the current real(value), then
     * the next imag(value) is smaller that the current image(value)*/
    for(i=0; i<N-1; i++)
        if(fabs(creal(sorted_vals[i])-creal(sorted_vals[i+1])) < 0.00001 )
            TEST_ASSERT_TRUE(cimag(sorted_vals[i])>=cimag(sorted_vals[i+1]));
}

void test__cmplxPairUp(void){
    int i;
    const int N = 36;
    double_complex vals[36] ={
        cmplx(1.0, 1.0), cmplx(7.0, 1.0),  cmplx(10.0, 5.0),
        cmplx(12.0, 4.0), cmplx(4.0, 4.0), cmplx(8.0, 0.0),
        cmplx(10.0, -1.0), cmplx(7.0, 5.0), cmplx(7.0, 2.0),
        cmplx(5.0, 1.0), cmplx(4.0, -1.0), cmplx(32.0, 3.0),
        cmplx(32.0, 32.5), cmplx(25.0, 0.0), cmplx(2.0, -2.0),
        cmplx(7.0, -2.0), cmplx(1.0, -1.0), cmplx(12.0, -1.0),
        cmplx(2.0, -1.0), cmplx(4.0, 2.0), cmplx(10.0, 6.0),
        cmplx(5.0, 0.0), cmplx(32.0, 1.5), cmplx(7.0, -10.0),
        cmplx(1.0, -1.5), cmplx(4.0, 25.0), cmplx(3.0, 2.0),
        cmplx(1.0, 0.0), cmplx(10.0, 5.0), cmplx(10.0, 2.0),
        cmplx(10.0, -3.5), cmplx(30.0, -10.0), cmplx(7.0, -12.0),
        cmplx(1.0, -13.5), cmplx(12.0, -12.0), cmplx(32.0, 23.0)
    };
    double_complex sorted_vals[36];

    /* Sort assending order */
    cmplxPairUp(vals, sorted_vals, N);

    /* Check that the next real(value) is either the same or greater than current real(value),
     * Ignoring purely real numbers */
    for(i=0; i<N-1; i++)
        if( !(fabs(cimag(sorted_vals[i])) < 0.0001) && !(fabs(cimag(sorted_vals[i+1])) < 0.0001) )
            TEST_ASSERT_TRUE(creal(sorted_vals[i])<=creal(sorted_vals[i+1]));

    /* Check that the next real(value) is either the same or greater than current real(value),
     * Only considering purely real numbers */
    for(i=0; i<N-1; i++)
        if( (fabs(cimag(sorted_vals[i])) < 0.0001) && (fabs(cimag(sorted_vals[i+1])) < 0.0001) )
            TEST_ASSERT_TRUE(creal(sorted_vals[i])<=creal(sorted_vals[i+1]));

    /* Check that if the next real(value) is the same as the current real(value), then
     * the next imag(value) is greater that the current image(value)
     * Ignoring purely real numbers */
    for(i=0; i<N-1; i++)
        if(fabs(creal(sorted_vals[i])-creal(sorted_vals[i+1])) < 0.00001 )
            if( !(fabs(cimag(sorted_vals[i])) < 0.0001) && !(fabs(cimag(sorted_vals[i+1])) < 0.0001) )
                TEST_ASSERT_TRUE(cimag(sorted_vals[i])<=cimag(sorted_vals[i+1]));
}

void test__getVoronoiWeights(void){
    int i, it, td, nDirs;
    float* dirs_deg, *weights;
    float sum, tmp, scale;

    /* Config */
    const float acceptedTolerance = 0.01f;
    const int nIterations = 100;

    /* Loop over T-designs */
    for(td=2; td<21; td++){
        dirs_deg = (float*)__HANDLES_Tdesign_dirs_deg[td];
        nDirs = __Tdesign_nPoints_per_degree[td];

        /* Compute weights */
        weights = malloc1d(nDirs*sizeof(float));
        getVoronoiWeights(dirs_deg, nDirs, 0, weights);

        /* Assert that they sum to 4PI */
        sum = sumf(weights, nDirs);
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, 4.0f*SAF_PI, sum);

        /* Due to the uniform arrangement, all the weights should be the same */
        for(i=1; i<nDirs; i++)
            TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, weights[0], weights[i]);

        /* clean-up */
        free(weights);
    }

    /* Loop over some random arrangement of points */
    for(it=0; it<nIterations; it++){
        rand_0_1(&tmp, 1);
        nDirs = (int)(tmp*190.0f + 10.0f); /* random number between 10..200 */

        /* Random dirs (-180..180 azi, -180..180 elev) */
        dirs_deg = malloc1d(nDirs*2*sizeof(float));
        rand_m1_1(dirs_deg, nDirs*2);
        scale = 180.0f;
        utility_svsmul(dirs_deg, &scale, nDirs*2, dirs_deg);

        /* Compute weights */
        weights = malloc1d(nDirs*sizeof(float));
        getVoronoiWeights(dirs_deg, nDirs, 0, weights);

        /* Assert that they sum to 4PI */
        sum = sumf(weights, nDirs);
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, 4.0f*SAF_PI, sum);

        /* clean-up */
        free(dirs_deg);
        free(weights);
    }
}

void test__unique_i(void){
    int i, nUnique;
    int* uniqueVals;
    int* uniqueInds;

    /* test1 */
    int input[6] = {1, 2, 2, 10, 11, 12};
    int uniqueVals_ref[5] = {1, 2, 10, 11, 12};
    int uniqueInds_ref[5] = {0, 2, 3, 4, 5};
    unique_i(input, 6, &uniqueVals, &uniqueInds, &nUnique);
    TEST_ASSERT_EQUAL(5, nUnique);
    for(i=0; i<nUnique; i++){
        TEST_ASSERT_EQUAL(uniqueVals_ref[i], uniqueVals[i]);
        TEST_ASSERT_EQUAL(uniqueInds_ref[i], uniqueInds[i]);
    }
    free(uniqueVals);
    free(uniqueInds);

    /* test2 */
    int input2[12] = {1, 10, 1, 3, 1, 3, 4, 7, 8, 10, 10, 2};
    int uniqueVals_ref2[7] = {1, 3, 4, 7, 8, 10, 2};
    int uniqueInds_ref2[7] = {4, 5, 6, 7, 8, 10, 11};
    unique_i(input2, 12, &uniqueVals, &uniqueInds, &nUnique);
    TEST_ASSERT_EQUAL(7, nUnique);
    for(i=0; i<nUnique; i++){
        TEST_ASSERT_EQUAL(uniqueVals_ref2[i], uniqueVals[i]);
        TEST_ASSERT_EQUAL(uniqueInds_ref2[i], uniqueInds[i]);
    }
    free(uniqueVals);
    free(uniqueInds);
}

void test__latticeDecorrelator(void){
    int c, band, nBands, idx, hopIdx, i;
    void* hDecor, *hSTFT;
    float icc, tmp, tmp2;
    float* freqVector;
    float** inputTimeDomainData, **outputTimeDomainData, **tempHop;
    float_complex*** inTFframe, ***outTFframe;

    /* config */
    const float acceptedICC = 0.05f;
    const int nCH = 24;
    const int nTestHops = 800;
    const int hopSize = 128;
    const int procDelay = hopSize*12 + 12;
    const int lSig = nTestHops*hopSize+procDelay;
    const float fs = 48e3f;
    nBands = hopSize+5;

    /* audio buffers */
    inputTimeDomainData = (float**) calloc2d(1, lSig, sizeof(float));
    outputTimeDomainData = (float**) calloc2d(nCH, lSig, sizeof(float));
    inTFframe = (float_complex***)malloc3d(nBands, nCH, 1, sizeof(float_complex));
    outTFframe = (float_complex***)malloc3d(nBands, nCH, 1, sizeof(float_complex));
    tempHop = (float**) malloc2d(nCH, hopSize, sizeof(float));

    /* Initialise afSTFT and input data */
    afSTFT_create(&hSTFT, 1, nCH, hopSize, 0, 1, AFSTFT_BANDS_CH_TIME);
    rand_m1_1(FLATTEN2D(inputTimeDomainData), 1*lSig); /* populate with random numbers */
    freqVector = malloc1d(nBands*sizeof(float));
    afSTFT_getCentreFreqs(hSTFT, fs, nBands, freqVector);

    /* setup decorrelator */
    int orders[4] = {20, 15, 6, 6}; /* 20th order up to 700Hz, 15th->2.4kHz, 6th->4kHz, 3rd->12kHz, NONE(only delays)->Nyquist */
    //float freqCutoffs[4] = {600.0f, 2.6e3f, 4.5e3f, 12e3f};
    float freqCutoffs[4] = {900.0f, 6.8e3f, 12e3f, 24e3f};
    const int maxDelay = 12;
    latticeDecorrelator_create(&hDecor, fs, hopSize, freqVector, nBands, nCH, orders, freqCutoffs, 4, maxDelay, 0, 0.75f);

    /* Processing loop */
    idx = 0;
    hopIdx = 0;
    while(idx<lSig-hopSize*2){
        for(c=0; c<1; c++)
            memcpy(tempHop[c], &(inputTimeDomainData[c][hopIdx*hopSize]), hopSize*sizeof(float));

        /* forward TF transform, and replicate to all channels */
        afSTFT_forward(hSTFT, tempHop, hopSize, inTFframe);
        for(band=0; band<nBands; band++)
            for(i=1; i<nCH;i++)
                inTFframe[band][i][0] = inTFframe[band][0][0];

        /* decorrelate */
        latticeDecorrelator_apply(hDecor, inTFframe, 1, outTFframe);

        /*  backward TF transform */
        afSTFT_backward(hSTFT, outTFframe, hopSize, tempHop);

        /* Copy frame to output TD buffer */
        for(c=0; c<nCH; c++)
            memcpy(&(outputTimeDomainData[c][hopIdx*hopSize]), tempHop[c], hopSize*sizeof(float));
        idx+=hopSize;
        hopIdx++;
    }

    /* Compensate for processing delay, and check that the inter-channel correlation
     * coefficient is below the accepted threshold (ideally 0, if fully
     * decorrelated...) */
    for(c=0; c<nCH; c++){
        utility_svvdot(inputTimeDomainData[0], &outputTimeDomainData[c][procDelay], (lSig-procDelay), &icc);
        utility_svvdot(inputTimeDomainData[0], inputTimeDomainData[0], (lSig-procDelay), &tmp);
        utility_svvdot(&outputTimeDomainData[c][procDelay], &outputTimeDomainData[c][procDelay], (lSig-procDelay), &tmp2);

        icc = icc/sqrtf(tmp*tmp2); /* normalise */
        TEST_ASSERT_TRUE(fabsf(icc)<acceptedICC);
    }
#if 0
    /* Check for mutually decorrelated channels... */
    int c2;
    for(c=0; c<nCH; c++){
        for(c2=0; c2<nCH; c2++){
            utility_svvdot(&outputTimeDomainData[c][procDelay], &outputTimeDomainData[c2][procDelay], (lSig-procDelay), &icc);
            utility_svvdot(&outputTimeDomainData[c2][procDelay], &outputTimeDomainData[c2][procDelay], (lSig-procDelay), &tmp);
            utility_svvdot(&outputTimeDomainData[c][procDelay], &outputTimeDomainData[c][procDelay], (lSig-procDelay), &tmp2);

            if (c!=c2){
                icc = icc/sqrtf(tmp*tmp2); /* normalise */
                TEST_ASSERT_TRUE(fabsf(icc)<acceptedICC);
            }
        }
    }
#endif

    /* Clean-up */
    latticeDecorrelator_destroy(&hDecor);
    free(inTFframe);
    free(outTFframe);
    free(tempHop);
    afSTFT_destroy(&hSTFT);
    free(inputTimeDomainData);
    free(outputTimeDomainData);
}

void test__butterCoeffs(void){
    int i;
    float fs, cutoff_freq, cutoff_freq2;
    int order;

    /* Config */
    const double acceptedTolerance = 0.00001f;

    /* 1st order Low-pass filter */
    fs = 48e3f;
    cutoff_freq = 3000.0f;
    order = 1;
    double a_test1[2], b_test1[2];
    butterCoeffs(BUTTER_FILTER_LPF, order, cutoff_freq, 0.0f, fs, (double*)b_test1, (double*)a_test1);
    const double a_ref1[2] = {1,-0.668178637919299};
    const double b_ref1[2] = {0.165910681040351,0.165910681040351};
    for(i=0; i<2; i++){ /* Compare with the values given by Matlab's butter function */
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, a_test1[i], a_ref1[i]);
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, b_test1[i], b_ref1[i]);
    }

    /* 2nd order Low-pass filter */
    fs = 48e3f;
    cutoff_freq = 12000.0f;
    order = 2;
    double a_test2[3], b_test2[3];
    butterCoeffs(BUTTER_FILTER_LPF, order, cutoff_freq, 0.0f, fs, (double*)b_test2, (double*)a_test2);
    const double a_ref2[3] = {1.0,-2.22044604925031e-16,0.171572875253810};
    const double b_ref2[3] = {0.292893218813452,0.585786437626905,0.292893218813452};
    for(i=0; i<3; i++){ /* Compare with the values given by Matlab's butter function */
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, a_test2[i], a_ref2[i]);
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, b_test2[i], b_ref2[i]);
    }

    /* 3rd order Low-pass filter */
    fs = 48e3f;
    cutoff_freq = 200.0f;
    order = 3;
    double a_test3[4], b_test3[4];
    butterCoeffs(BUTTER_FILTER_LPF, order, cutoff_freq, 0.0f, fs, (double*)b_test3, (double*)a_test3);
    const double a_ref3[4] = {1.0,-2.94764161678340,2.89664496645376,-0.948985866903327};
    const double b_ref3[4] = {2.18534587909103e-06,6.55603763727308e-06,6.55603763727308e-06,2.18534587909103e-06};
    for(i=0; i<4; i++){ /* Compare with the values given by Matlab's butter function */
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, a_test3[i], a_ref3[i]);
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, b_test3[i], b_ref3[i]);
    }

    /* 6th order Low-pass filter */
    fs = 48e3f;
    cutoff_freq = 1e3f;
    order = 6;
    double a_test4[7], b_test4[7];
    butterCoeffs(BUTTER_FILTER_LPF, order, cutoff_freq, 0.0f, fs, (double*)b_test4, (double*)a_test4);
    const double a_ref4[7] = {1,-5.49431292177096,12.5978414666894,-15.4285267903275,10.6436770055305,-3.92144696766748,0.602772146971300};
    const double b_ref4[7] = {6.15535184628202e-08,3.69321110776921e-07,9.23302776942303e-07,1.23107036925640e-06,9.23302776942303e-07,3.69321110776921e-07,6.15535184628202e-08};
    for(i=0; i<7; i++){ /* Compare with the values given by Matlab's butter function */
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, a_test4[i], a_ref4[i]);
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, b_test4[i], b_ref4[i]);
    }

    /* 3rd order High-pass filter */
    fs = 48e3f;
    cutoff_freq = 3000.0f;
    order = 3;
    double a_test5[4], b_test5[4];
    butterCoeffs(BUTTER_FILTER_HPF, order, cutoff_freq, 0.0f, fs, (double*)b_test5, (double*)a_test5);
    const double a_ref5[4] = {1,-2.21916861831167,1.71511783003340,-0.453545933365530};
    const double b_ref5[4] = {0.673479047713825,-2.02043714314147,2.02043714314147,-0.673479047713825};
    for(i=0; i<4; i++){ /* Compare with the values given by Matlab's butter function */
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, a_test5[i], a_ref5[i]);
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, b_test5[i], b_ref5[i]);
    }

    /* 4th order High-pass filter */
    fs = 48e3f;
    cutoff_freq = 100.0;
    order = 4;
    double a_test6[5], b_test6[5];
    butterCoeffs(BUTTER_FILTER_HPF, order, cutoff_freq, 0.0f, fs, (double*)b_test6, (double*)a_test6);
    const double a_ref6[5] = {1.0,-3.96579438007005,5.89796693861409,-3.89854491737242,0.966372387692057};
    const double b_ref6[5] = {0.983042413984288,-3.93216965593715,5.89825448390573,-3.93216965593715,0.983042413984288};
    for(i=0; i<5; i++){ /* Compare with the values given by Matlab's butter function */
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, a_test6[i], a_ref6[i]);
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, b_test6[i], b_ref6[i]);
    }

    /* 2nd order Band-pass filter */
    fs = 48e3f;
    cutoff_freq = 100.0;
    cutoff_freq2 = 400.0;
    order = 2;
    double a_test7[5], b_test7[5];
    butterCoeffs(BUTTER_FILTER_BPF, order, cutoff_freq, cutoff_freq2, fs, (double*)b_test7, (double*)a_test7);
    const double a_ref7[5] = {1.0,-3.94312581006024,5.83226704209421,-3.83511871130750,0.945977936232284};
    const double b_ref7[5] = {0.000375069616051004,0.0,-0.000750139232102008,0.0,0.000375069616051004};
    for(i=0; i<5; i++){ /* Compare with the values given by Matlab's butter function */
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, a_test7[i], a_ref7[i]);
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, b_test7[i], b_ref7[i]);
    }

    /* 3rd order Band-stop filter */
    fs = 48e3f;
    cutoff_freq = 240.0;
    cutoff_freq2 = 1600.0;
    order = 3;
    double a_test9[7], b_test9[7];
    butterCoeffs(BUTTER_FILTER_BSF, order, cutoff_freq, cutoff_freq2, fs, (double*)b_test9, (double*)a_test9);
    const double a_ref9[7] = {1,-5.62580309774365,13.2124846784594,-16.5822627287366,11.7304049556188,-4.43493124452282,0.700107676775329};
    const double b_ref9[7] = {0.836724592951539,-5.00379660039217,12.4847741945760,-16.6354041344203,12.4847741945760,-5.00379660039217,0.836724592951539};
    for(i=0; i<7; i++){ /* Compare with the values given by Matlab's butter function */
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, a_test9[i], a_ref9[i]);
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance, b_test9[i], b_ref9[i]);
    }
}

void test__faf_IIRFilterbank(void){
    void* hFaF;
    int i, band;
    float* inSig, *outSig;
    float** outFrame, **outSig_bands;
    void* hFFT;
    float_complex* insig_fft, *outsig_fft;

    /* Config */
    const float acceptedTolerance_dB = 0.5f;
    const int signalLength = 256;
    const int frameSize = 256;//16;
    float fs = 48e3;
    int order = 3;
    float fc[6] = {176.776695296637f, 353.553390593274f, 707.106781186547f, 1414.21356237309f, 2828.42712474619f, 5656.85424949238f};
    inSig = malloc1d(signalLength * sizeof(float));
    outSig_bands = (float**)malloc2d(7, signalLength, sizeof(float));
    outSig = calloc1d(signalLength, sizeof(float));

    insig_fft = malloc1d((signalLength / 2 + 1) * sizeof(float_complex));
    outsig_fft = malloc1d((signalLength / 2 + 1) * sizeof(float_complex));

    /* Impulse */
    memset(inSig, 0, signalLength*sizeof(float));
    inSig[0] = 1.0f;

    /* Pass impulse through filterbank */
    outFrame = (float**)malloc2d(7, frameSize, sizeof(float));
    faf_IIRFilterbank_create(&hFaF, order, (float*)fc, 6, fs, frameSize);
    for(i=0; i< signalLength/frameSize; i++){
        faf_IIRFilterbank_apply(hFaF, &inSig[i*frameSize], outFrame, frameSize);
        for(band=0; band<7; band++)
            memcpy(&outSig_bands[band][i*frameSize], outFrame[band], frameSize*sizeof(float));
    }
    faf_IIRFilterbank_destroy(&hFaF);

    /* Sum the individual bands */
    for(band=0; band<7; band++)
        cblas_saxpy(signalLength, 1.0f, outSig_bands[band], 1, outSig, 1);

    /* Check that the magnitude difference between input and output is below 0.5dB */
    saf_rfft_create(&hFFT, signalLength);
    saf_rfft_forward(hFFT, inSig, insig_fft);
    saf_rfft_forward(hFFT, outSig, outsig_fft);
    for(i=0; i<signalLength/2+1; i++)
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance_dB, 0.0f, 20.0f * log10f(cabsf( ccdivf(outsig_fft[i],insig_fft[i]) )));

    /* Now the same thing, but for 1st order */
    order = 1;
    faf_IIRFilterbank_create(&hFaF, order, (float*)fc, 6, fs, frameSize);
    for(i=0; i< signalLength/frameSize; i++){
        faf_IIRFilterbank_apply(hFaF, &inSig[i*frameSize], outFrame, frameSize);
        for(band=0; band<7; band++)
            memcpy(&outSig_bands[band][i*frameSize], outFrame[band], frameSize*sizeof(float));
    }
    faf_IIRFilterbank_destroy(&hFaF);
    memset(outSig, 0, signalLength*sizeof(float));
    for(band=0; band<7; band++)
        cblas_saxpy(signalLength, 1.0f, outSig_bands[band], 1, outSig, 1);
    saf_rfft_forward(hFFT, outSig, outsig_fft);
    for(i=0; i<signalLength/2+1; i++)
        TEST_ASSERT_FLOAT_WITHIN(acceptedTolerance_dB, 0.0f, 20.0f * log10f(cabsf(ccdivf(outsig_fft[i], insig_fft[i]))));

    /* clean-up */
    saf_rfft_destroy(&hFFT);
    free(outFrame);
    free(inSig);
    free(outSig_bands);
    free(outSig);
    free(insig_fft);
    free(outsig_fft);
}

void test__gexpm(void){
    int i, j;
    float outM[6][6];

    /* prep */
    const float acceptedTolerance = 0.0001f;
    const float inM[6][6] = {
        {-0.376858200853762f,0.656790634216694f,0.124479178614046f,-0.334752428307223f,1.50745241578235f,0.0290651989052969f},
        {0.608382058262806f,0.581930485432986f,3.23135406998058f,-0.712003744668929f,-1.33872571354702f,-0.334742482743222f},
        {-0.795741418256672f,0.690709474622409f,0.620971281129248f,1.38749471231620f,0.897245329198841f,-0.0693670166113321f},
        {0.179789913109994f,-1.06135084902804f,-1.10032635271188f,0.612441344250358f,-2.43213807790664f,-0.479265889956047f},
        {-0.277441781278754f,-0.0732116130293688f,-0.572551795688137f,1.02024767389969f,0.167385894565923f,1.45210312619277f},
        {-0.205305770089918f,-1.59783032780633f,1.08539265129120f,0.460057585947626f,-1.02420974042838f,1.04117461500218f}
    };
    const float outM_ref[6][6] = {
        {0.385163650730121f,0.0865151585709784f,0.898406722231524f,0.877640791713973f,0.435244824708340f,0.888866982998854f},
        {-0.664938511314777f,5.02943129352875f,8.24444951891833f,2.23840978101979f,-0.942669833528886f,-2.38535530623266f},
        {-0.388189314743059f,0.429308537172675f,1.13870842882926f,1.60875776611798f,-1.44249911796405f,-1.51822150286392f},
        {1.05630187656688f,0.256606570814868f,-2.42701873560847f,-1.42372526577009f,-0.335273289873574f,-1.94362909671742f},
        {0.0261470437116839f,-3.03329326250434f,-3.50207776203591f,0.412043775125377f,-0.536000387729306f,1.61801775548557f},
        {-0.292024827617294f,-4.31537192033477f,-3.99160103133879f,0.312499067924889f,-1.46924802440347f,1.98522802303672f}
    };

    /* Compute matrix exponential */
    gexpm((float*)inM, 6, 0, (float*)outM);

    /* Check that output of SAF's gexpm, is similar to Matlab's expm: */
    for(i=0; i<6; i++)
        for(j=0; j<6; j++)
            TEST_ASSERT_TRUE( fabsf(outM[i][j] - outM_ref[i][j]) <= acceptedTolerance );
}
