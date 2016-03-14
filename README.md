# RFeditor
Computer codes for RFeditor program.
Available to the public.

Developer: Xiaotao Yang (add auto editing methods), Gary Pavlis (prototype of manual editing procedures), Yinzhi Wang (contributed to development of the autoediting methods through discussions and test use of the program).

Contact: xtyang@indiana.edu (Xiaotao Yang)

Reference for the method: X. Yang, G. Pavlis, Y. Wang (in revision). A Quality Control Method for Teleseismic P-Wave Receiver Functions, Bulletin of the Seismological Society of America

1. Current version: 3.6.1.

2. Source tree:

libtreditoperator
		This directory contains the lib needed by RFeditor editing procedures. However, the library files are currently embeded in RFeditor core codes. Thus compiling RFeditor does not require the compling of this library. This library can be used by other utilities or extensional programs.

RFeditor_corecode
		This folder contains all of the RFeditor source code.
		
Utilities
		This folder includes the utilities related to the use of RFeditor.

3. Dependency: See install.txt in the RFeditor_corecode directory (e.g., RFeditor3.6.1) for details on library dependency.

4. User Manual: detailed user manual is to be added later.
