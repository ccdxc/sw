// concatenated string versions
#define xstr(s) str(s)
#define str(s) #s

#define IONIC_MP_VERSION_EXTENSION				xstr(IONIC_VERSIONINFO_EXTENSION)
#define IONIC_MP_PRODUCT_NAME                   "Ionic Ndis Miniport Driver " 
#define IONIC_MP_DRIVER_NAME                    "Ionic64.sys" // xstr(IONIC_DRIVER_NAME)
#define IONIC_MP_VERSION_STRING	                xstr(IONIC_MAJOR_DRIVER_VERSION) "." xstr(IONIC_MINOR_DRIVER_VERSION) "." xstr(IONIC_SP_DRIVER_VERSION) "." xstr(IONIC_BUILD_DRIVER_VERSION)
#define IONIC_MP_PRODVERSION_STRING             xstr(IONIC_MAJOR_DRIVER_VERSION) "." xstr(IONIC_MINOR_DRIVER_VERSION) "." xstr(IONIC_SP_DRIVER_VERSION)

#define IONIC_MP_VENDOR_DRIVER_VERSION          ((IONIC_MAJOR_DRIVER_VERSION << 16) | IONIC_MINOR_DRIVER_VERSION)


// NDIS version picked up from the build
#if defined(NDIS60_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION             0
#elif defined(NDIS620_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION            20
#elif defined(NDIS630_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION            30
#elif defined(NDIS640_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION            40
#elif defined(NDIS650_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION            50
#elif defined(NDIS660_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION            60
#elif defined(NDIS670_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION            70
#elif defined(NDIS680_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION            80
#elif defined(NDIS681_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION            81
#elif defined(NDIS682_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION            82
#elif defined(NDIS683_MINIPORT)
#define IONIC_MP_NDIS_MAJOR_VERSION             6
#define IONIC_MP_NDIS_MINOR_VERSION            83
#else

#error Unsupported NDIS version

#endif

// Version Info structure for holding global versioning data
#define REVISION_MAX_STR_SIZE	64
// global VersionInfo structure
typedef struct _IONIC_VERSION_INFO {
	USHORT				VerMaj;   				// driver version: major
	USHORT				VerMin;   				// driver version: minor
	USHORT				VerSP;    				// driver version: SP
	ULONG				VerBuild; 				// driver version: build
	CHAR				VerString[REVISION_MAX_STR_SIZE];	// driver version string
	WCHAR				VerStringW[REVISION_MAX_STR_SIZE];	// driver version string
} IONIC_VERSION_INFO, * PIONIC_VERSION_INFO;
