CGIScaler
=========
Jakub Pastuszek <jpastuszek@gmail.com>
v2.1, Feb 2010

Fast, small, ImageMagick® based CGI image scaler for any web based application requiring intensive thumbnailing.

For installation instructions and setup examples see link:INSTALL.html[INSTALL].

## Theory of operation

This program utilises ImageMagick for image access and manipulation.
For each request CGI process is spawned.
Firstly CGIScaler will check if original and cached versions of requested thumbnail exist.
If original does not exist cached image will be removed and error image will be sent.
If cached thumbnail exist and it has the same time stamp as original file the cached thumbnail will be immediately sent and no ImageMagick library initialization will happen.
If cached thumbnail timestamp is older than timestamp of the original image the cached thumbnail will be deleted.
If cached thumbnail does not exist (or was just deleted) ImageMagick will be initialized and scaled down version of original image created.
CGIScaler will do fast pre-scaling or load smaller sub image (in case of JPEG) before performing high quality scale-down.
To further improve timing CGIScaler will sent scaled down image to the client before writing it down to disc cache.
After image was sent to client it is saved in cache directory by replicating sub directory structure and with use of file name in format: '<original_file_name_with_extension>-<width>-<height>-<strict>-<quality>.<output_extension>'.
In case of any error that would happen before thumbnail was sent to the client error image will be sent instead.

## Scaling modes

Currently two modes are supported:

### Fit
Image will be scaled to fit to required dimensions without loosing aspect ratio.
In this mode one of the resulting image dimension may be smaller then requested.

image:doc/fit.jpg[Fit example image]

### Strict
In strict mode image will be cropped and scaled to fill required dimensions.

image:doc/strict.jpg[Strict example image]

## Configuration

For configuration parameters see ./cgiscaler --help or see installation link:INSTALL.html[INSTALL] file.

## Performance

Performance tests show that CGIScaler is capable of delivering 39.7 thumbanils per second with empty cache and 126.2 per second when all thumbnails are cached on virtual server with only one VCPU (Xen).

For more details see link:PERFORMANCE.html[PERFORMANCE] file.

## End Note

I hope that this program will be useful.
Any suggestions and ideas are welcome.

Code and binary releases are available at:
http://sourceforge.net/projects/cgiscaler/

Jakub Pastuszek <jpastuszek@gmail.com>

This program comes with and uses CGreen library, though final binary does not link or require it to run.
CGreen is written by Marcus Baker and licensed under LGPL - see cgreen/LICENSE for licensing details.

