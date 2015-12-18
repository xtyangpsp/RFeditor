Software dependency.

The following libraries are required in order to compile RFeditor (current version 3.0):
1. seisw: this libraries is used in plotting seismic wiggle lines. It is by default installed/compiled when you install antelope.

2. gsl: used in doing FFT during editing procedure: kill low-frequency contaminated traces;

3. boost: This is a separated library that the user need to install from third party website: http://www.boost.org

4. xmotif: xwindow functions; On Mac OSX, it can be installed through fink. You may find some useful information on: http://motif.ics.com/motif

5. SEISPP: this is a standard library included in Antelope contrib package. It should be installed by default when you compile your contrib programs.

Noted by Xiaotao Yang, Dec 17, 2015
