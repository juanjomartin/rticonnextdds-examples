Applies to RTI Data Distribution Service 4.0 and above.

Purpose
=======
PUBLISHER:
In the 'Asynchronous Publishing' we showed how asynchronous publication is useful to aggregate small messages into fewer packets, and thus achieve lower overhead. In this example we present another use case.

Here, we send a burst of large samples once per second. If many writers are sending in such bursts (particularly if synchronized), this will increase collisions, forcing resends in the case of reliable transmission, or causing data loss for best-effort transmissions.

We use asynchronous publication with a custom flowcontroller to smooth out these bursts.

SUBSCRIBER:
No changes are required on the subscriber, but we add a clock to show the effects of the flowcontroller.

Building
=======
Make sure you are using one of the relevant RTI Data Distribution Service versions, as specified at the top of the Solution.

Before compiling or running the example, make sure the environment variable NDDSHOME is set to the directory where your version of RTI Data Distribution Service is installed.

Run rtiddsgen with the -example option and the target architecture of your choice (for example, i86Win32VS2005). The RTI Data Distribution Service Getting Started Guide describes this process in detail. Follow the same procedure to generate the code and build the examples. Do not use the -replace option.

After running rtiddsgen like this...

C:\local\Custom_Flowcontroller\c++> rtiddsgen -language C++ -example i86Win32VS2005 cfc.idl

...you will see messages that look like this:

File C:\local\Custom_Flowcontroller\c++\cfc_subscriber.cxx already exists and will not be replaced with updated content. If you would like to get a new file with the new content, either remove this file or supply -replace option.
File C:\local\Custom_Flowcontroller\c++\cfc_publisher.cxx already exists and will not be replaced with updated content. If you would like to get a new file with the new content, either remove this file or supply -replace option.
File C:\local\Custom_Flowcontroller\c++\USER_QOS_PROFILES.xml already exists and will not be replaced with updated content. If you would like to get a new file with the new content, either remove this file or supply -replace option.

This is normal and is only informing you that the subscriber/publisher code has not been replaced, which is fine since all the source files for the example are already provided.

Running
=======
In two separate command prompt windows for the publisher and subscriber, navigate to the objs/<arch> directory and run these commands:

Windows systems:

    * cfc_publisher.exe <domain#> 4
    * cfc_subscriber.exe <domain#> 5

UNIX systems:

    * ./cfc_publisher <domain#> 4
    * ./cfc_subscriber <domain#> 5

The applications accept two arguments:

   1. The <domain #>. Both applications must use the same domain # in order to communicate. The default is 0.
   2. How long the examples should run, measured in samples for the publisher and sleep periods for the subscriber. A value of '0' instructs the application to run forever; this is the default.

While generating the output below, we used values that would capture the most interesting behavior.

Publisher Output
=============
Writing cfc, sample 0
Writing cfc, sample 1
Writing cfc, sample 2
Writing cfc, sample 3
Writing cfc, sample 4
Writing cfc, sample 5
Writing cfc, sample 6
Writing cfc, sample 7
Writing cfc, sample 8
Writing cfc, sample 9
Writing cfc, sample 10
Writing cfc, sample 11
Writing cfc, sample 12
Writing cfc, sample 13
Writing cfc, sample 14
Writing cfc, sample 15
Writing cfc, sample 16
Writing cfc, sample 17
Writing cfc, sample 18
Writing cfc, sample 19
Writing cfc, sample 20
Writing cfc, sample 21
Writing cfc, sample 22
Writing cfc, sample 23
Writing cfc, sample 24
Writing cfc, sample 25
Writing cfc, sample 26
Writing cfc, sample 27
Writing cfc, sample 28
Writing cfc, sample 29
Writing cfc, sample 30
Writing cfc, sample 31
Writing cfc, sample 32
Writing cfc, sample 33
Writing cfc, sample 34
Writing cfc, sample 35
Writing cfc, sample 36
Writing cfc, sample 37
Writing cfc, sample 38
Writing cfc, sample 39

Subscriber Output
==============
@ t=0.43s, got x = 10
@ t=0.43s, got x = 11
@ t=0.54s, got x = 12
@ t=0.54s, got x = 13
@ t=0.66s, got x = 14
@ t=0.66s, got x = 15
@ t=0.77s, got x = 16
@ t=0.77s, got x = 17
@ t=0.88s, got x = 18
@ t=0.88s, got x = 19
@ t=1.43s, got x = 20
@ t=1.43s, got x = 21
@ t=1.54s, got x = 22
@ t=1.54s, got x = 23
@ t=1.65s, got x = 24
@ t=1.65s, got x = 25
@ t=1.77s, got x = 26
@ t=1.77s, got x = 27
@ t=1.88s, got x = 28
@ t=1.88s, got x = 29
@ t=2.43s, got x = 30