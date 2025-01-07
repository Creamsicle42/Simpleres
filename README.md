# Simpleres

Simple resource pack implementation in C

## About

Simpleres is a general purpose library for reading and managing arbitrary resource files from a single pack file. Simpleres handles the loading of a resource pack, loading of compressed subresources into a static memory space. And automatic freeing of resource memory when not used.

## SMR Files

Simpleres works with .smr pack files, also known as Simpleres files. Simpleres files consist of a header section, followed by all resource chunks in sequence. All values within the .smr standard are encoded in big-endian format.

### The SMR Header

The .smr header section consists of...

1. 4 bytes encoding "smpr" in binary
2. 2 bytes encoding the standard version used.
3. 2 bytes encoding the number of resources in the smr pack.
4. 4 bytes encoding the length of the resource id section.
5. The resource ID section, in which the names of all files within the .smr pack are encoded as raw ASCII strings with no terminator.
6. The resource header section, in which all resource headers are stored in order.

Resource headers consist of...

1. 4 bytes encoding the start position of the resource name from the start of the id section.
2. 2 bytes encoding the length of the resource name.
3. 2 bytes containing flags relevant to the resource.
4. 4 bytes encoding the start position of the resource data from the start of the file.
4. 4 bytes encoding the length of the resource data.

### SMR Resource Data

SMR Resources are located as raw bytes directly after the header. Resources are assumed to be LZW compressed unless the SMR_UNCOMPRESSED flag is set in the header for the given resource.

## Usage

To read resources from a .smr file, the file must first be loaded by initializing a `SMR_ResourcePack`.

```c
int data_size = 500'000; // Only 500kb of data can ever be used by the resource pack
char *data = malloc(data_size);
SMR_ResourcePack pack;

int error = SMR_ResourcePackInit(
    &pack, // Pointer to the pack struc
    "pack.smr", // Path to the resource pack
    data_space, // Space that will be used to store resource data
    data_size // Size of allocated data space
);

if (error != SMR_OK) {
    // Code to handle a resource pack failure
}
```

To get a hande to a resource within a loaded pack...

```c
SMR_ResourceHandle handle;
int error = SMR_GetResourceHandle(
    &pack, // Pack that has the resource
    &handle, // Handle that will point to the resource
    "resource.txt" // ID of the resource, will be a filename for most packer implementations
);
```

To use a handle to access resource data...

```c
SMR_ResourceSlice slice;
int error = SMR_GetResourceSlice(
    &pack,
    &handle,
    &slice
);
```

And finally to free a resource...

```c
SMR_ReturnResourceHandle(
    &pack,
    handle
);
```

Once all handles to a resource are returned, the resource will automatically be freed.
