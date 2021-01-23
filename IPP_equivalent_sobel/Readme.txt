This package contains the source, header files needed for building Sobel filter.
ALso Images for running the Sobel filter are included. A performance comparison sheet is also included.
The package comes with a pre-built binary (Denoise.exe) located in the Images folder.

Pre-requisites for Running:
	1) AMD Catalyst driver
	2) Visual Studio 2012 runtime 
	
Steps to run the exe:
	1) Go to the command prompt
	2) cd to the "Images" folder
	3) Run Denoise.exe with a grayscale image in bmp format.
		For e.g Denoise.exe lena.bmp.
	4) Output image will be in the Images folder. The output file names are OCL.bmp
		in the case of AMD platforms and IPP.bmp in case of Intel platforms with IPP.
	5) Time taken for processing the image is printed on the console.
Example: 
	Denoise.exe lena.bmp

Pre-requisites for Building:
	Intel platform
		1. IPP run time should be installed
		2. Visual Studio 2012 

	AMD platform
		1. Visual Studio 2012 
		2. AMD catalyst driver
		3. AMD APP SDK

Steps to compile and build the project:

	1) Open the .sln file in Visual Studio 2012.
	2) If AMD APP SDK is not installed then add the path to appropriate OpenCL header files.
	3) Change the configuration to Relase | x64.
	4) Build the project.
	5) When the build succeeds the Denoise.exe and separableFilter.cl files are copied to the Images folder
	6) Images folder is the working folder which contains the input images in the bmp format.
	






