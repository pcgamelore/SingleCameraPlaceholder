# SingleCameraPlaceholder
Sample code the read Jetson Tx1/Tx2 cameras, encode, process the frame, and encode processed output.

We read frame from camera and put them in a Q. Jetson Argus libraries are used to read frames. The Q, is popped by Open CV Consumer and the subsequently processed by opencv routines. Rationale behind using the Q is to provided temporary storage facility for input frames, if their open CV consumers takes longer than a frame duration to process them. 

Frames which are consumed by open cv consumer are also fed to Jetson hardware encoder for purpose of recording, input video. The processed output of opencv consumer is subsequently read by Jetson hardware encoder for purpose of recording output video. Encoder is used in time sliced manner, for encoding input and output videos. 

Gstreamer, encode pipelines are used to encode input and output videos. Input recording channel uses nveglstreamsrc as frame source and output Gstreamer pipeline uses Gstreamer element 'appsrc' as frame source. 'appsrc' is fed by output of openCV consumer.
