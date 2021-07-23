# P-SPot Extractor

## About
---
Little and simple utility to unzip a zipfile with options stored in a config file. This is based on Tipster's UnzipUnrar source code. 

The following changes have been made from the original source code:
* A line linking to the original source archive has been modified to direct to this GitHub repository.
* Built EBOOT and configuration file ([unzip.cfg](bin/unzip.cfg)) exist in the `bin` directory, for cleanliness.
* Several compilation warnings were fixed, the source should now compile without complaint with GCC.

The original, unmodified source code can be found within commit [b917ece](https://github.com/PSP-Archive/P-SPot_Extractor/commit/b917ecee1eccc3c5b01b9214649b00dec7f01f8a). It is confirmed to work on the latest version of the PSPSDK at the time of writing.

## Usage
---
P-Spot Extractor functions based on the [unzip.cfg](bin/unzip.cfg) configuration file. It has the following parameters:
* `ZIPFILE`: The path to the zip file archive.
* `APPNAME`: Application name, printed to the screen on program execution.
* `DESTDIR`: The path of where to extract the archive to.
* `BYTESNEEDED`: The size of the zip file archive, in bytes.
* `AUTODELETE`: Determines whether P-spot Extractor will delete itself upon finishing the task. Useful for temporary installers.

## Building
---
An EBOOT.PBP binary will be build to `bin/` with the use of `make install` in the root of this repository. Requires pspsdk.

## Licensing
---
No license was provided with P-Spot Extractor upon it's release. However, it contains `unzip` from the zlib library, which is bound by the [zlib/libpng license](https://opensource.org/licenses/Zlib). The license is also found in [unzip.h](unzip.h).