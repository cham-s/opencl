#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PROGRAM_FILE "add_numbers.cl"
#define KERNEL_FUNC "add_numbers"
#define ARRAY_SIZE 64;

#ifdef MAC
# include <OpenCL/cl.h>
#else
# include <CL/cl.h>
#endif

// find a GPU or CPU associated with the first platform
cl_device_id	create_device(void)
{
	cl_platform_id	platform;
	cl_device_id	dev;
	int				err;

	//ID the platform
	err = clGetPlatformIDs(1, &platform, NULL);
	if(err < 0)
	{
		perror("Couldn't identify a platform");
		exit(EXIT_FAILURE);
	}

	//access a device
	err = clGgetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
	if (err == CL_DEVICE_NOT_FOUND)
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
	if (err < 0)
	{
		perror("Couldn't identify a platform");
		exit(EXIT_FAILURE);
	}
	return (dev);
}

cl_program	buil_program(cl_context ctx, cl_device_id, const char * filename)
{
	cl_program	program;
	FILE		*program_handle;
	char		*program_buffer;
	char		*program_log;
	size_t		program_size;
	size_t		log_size;
	int			err;

	// read program file and place content into buffer
	program_handle = fopen(filename, "r");
	if (program_handle == NULL)
	{
		perror("Couldn't find the program file");
		exit(2);
	}
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char *)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);

	// create program from file
	program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
	if (err < 0)
	{
		perror("Couldn't create the program");
		exit(1);
	}
	free(program_buffer);

	// build program
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err < 0)
	{
		// find size of log and print to std output
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		program_log = (char *)malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
		//printf();
		printf("%s\n", program_log);
		free(program_log);
		exit(1);
	}
	return program;
}

int main()
{
	//opencl structures
	cl_device_id		device;
	cl_context			context;
	cl_program			program;
	cl_kernel			kernel;
	cl_command_queue	queue;
	cl_int				i;
	cl_int				j;
	cl_int				err;
	size_t				local_size;
	size_t				global_size;

	// data and buffers
	float				data[ARRAY_SIZE];
	float				sum[2];
	float				total;
	float				actual_sum;
	cl_mem				input_buffer;
	cl_mem				sum_buffer;
	cl_int				num_groups;

	// init data
	for (i=0; i<ARRAY_SIZE; i++)
		data[i] = 1.0*i;

	//create device and context
	device = create_device();
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0)
	{
		perror("Couldn't create a context");
		exit(1);
	}

	// build program
	program = build_program(context, device, PROGRAM_FILE);
	
	// create a data buffer
	global_size = 8;
	local_size = 4;
	num_groups = global_size/local_size;
	input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY |
			CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), data, &err);
	sum_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE |
			CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), sum, &err);
	if (err < 0)
	{
		perror("Couldn't create a buffer");
		exit(1);
	}

	// create a command queue 
	queue = clCreateCommandQueue(context, devide, 0, &err);
	if (err < 0)
	{
		perror("Couldn't create a command queue");
		exit(1);
	}

	// create kernel arguments
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
	err |= clSetKernelArg(kernel, 1, local_size * sizeof(float), NULL);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &sum_buffer);
}
