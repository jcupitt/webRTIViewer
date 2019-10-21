[this repository is a development copy of the code at
http://vcg.isti.cnr.it/~palma/webRTIViewer.zip ... any changes here are
expected to be folded back into the master distribution, see CHANGELOG.md
for a summary of differences]

## webRTIViewer

WebRTIViewer is a viewer of high resolution RTI images (LRGB-PTM, RGB-PTM,
HSH). for the web. It allows also the visualization of common images in
the format JPEG, PNG and TIF. It is available under the GNU General Public
License version 3.

In order to show the image on the web, it must be preprocess with the
command line  tool webGLRTIMaker that creates the multi-resolution format
to load in your web server.  In the folders `webGLRTIMaker-win-x86` and
`webGLRTIMaker-win-x64` there are the 32 and 64 bit versions of the tool
for Windows. The folder `webGLRTIMaker-src` contains the source code. It
requires a recent version of the framework QT.

### Building in Linux

You can build with qmake or autotools.

For qmake, use the following steps:

```shell
# System information.
$ uname -a
Linux x220 4.16.0-2-amd64 #1 SMP Debian 4.16.12-1 (2018-05-27) x86_64 GNU/Linux

# Install dependencies.
$ sudo apt update
$ sudo apt install qt5-default

# Verify installation of QT5/QMake
$ qmake -v
QMake version 3.1
Using Qt version 5.10.1 in /usr/lib/x86_64-linux-gnu

# Clone webRTIViewer from Github
$ git clone git@github.com:jcupitt/webRTIViewer.git
$ cd webGLRTIMaker-src
$ mkdir build
$ cd build
$ qmake ../webGLRtiMaker.pro
$ make

# Optionally, install executable in /usr/local/bin
$ sudo cp webGLRtiMaker /usr/local/bin
```

For autotools, use:

```
# Install dependencies.
$ sudo apt update
$ sudo apt install qt5-default

# Clone webRTIViewer from Github
$ git clone git@github.com:jcupitt/webRTIViewer.git
$ cd webRTIViewer
$ ./boostrap.sh
$ ./configure
$ make

# Optionally, install executable in /usr/local/bin
$ sudo make install
```

## PREPROCESSING 

To preprocess the image you must execute the following command line:

```bash
webGLRTIMaker.exe pathtothefile -q 90
```

where we have the filepath of the image and a quality value (in example 90).
The quality can accept value in the range 0-100 (100 = maximum quality).
The tool creates a new folder (with the same name of the image) in the
folder of the image with all the data to store in the web server. At this
point you can copy the new folder in your web server.

## VIEWER 

The folder `webViewer` contains the code of the HTML5 RTIViewer. To simply
show an image you must add the following code in your HTML page:

```html
<div id="viewerContainer">
    <script  type="text/javascript">
        createRtiViewer("viewerContainer", "webrti", 900, 600); 
    </script>
</div>
```
	
where the function `createRtiViewer()` takes the following parameters:

1 the id of the div tag that must contain the viewer (`viewerContainer`
in the example);

2 the path to the folder that contains the proprocessed image (`webrti`
in the example);

3 the size in pixels (width and height) of the viewport of the viewer
(900 and 600 in the example)

## ACKNOWLEDGEMENT 

If you use the viewer in your site please send me an email
(gianpaolo.palma@isti.cnr.it) with the link of your site. 
