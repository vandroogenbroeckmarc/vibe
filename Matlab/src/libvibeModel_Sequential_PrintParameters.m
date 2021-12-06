% COPYRIGHT Pierard Sebastien, January 2019
%
% This file is part of a library that aims at providing a matlab (not octave)
% implementation of ViBe (for using it, pay attention to the fact that
% it is a patented method, see comments below) and demonstrating how an
% implementation of ViBe can be vectorized. Whereas ViBe can be extremely
% fast, this library has not been optimized for speed.
%
% ViBe is a patented algorithm of background subtraction. This means that
% its purpose is to label each pixel of all frames of any video stream as
% either "background" or "foreground". In two words, the foreground
% includes all the elements of the scene that are moving. More information
% about ViBe and the corresponding patents can be found on the webpage
% http://www.telecom.ulg.ac.be/research/vibe/.
% 
% Permission to use ViBe without payment of fee is granted for nonprofit
% educational and research purposes only. This work may not be copied or
% reproduced in whole or in part for any purpose. Copying, reproduction,
% or republishing for any purpose shall require a license. Please contact
% the author in such cases. All the code is provided without any guarantee.

function [] = libvibeModel_Sequential_PrintParameters ( model )
	fprintf ( 'Using ViBe background subtraction algorithm\n' ) ;
	fprintf ( ' - Number of samples per pixel:     %d\n' , libvibeModel_Sequential_GetNumberOfSamples ( model ) ) ;
	fprintf ( ' - Number of matches needed:        %d\n' , libvibeModel_Sequential_GetMatchingNumber ( model ) ) ;
	fprintf ( ' - Matching threshold:              %d\n' , libvibeModel_Sequential_GetMatchingThreshold ( model ) ) ;
	fprintf ( ' - Model update subsampling factor: %d\n' , libvibeModel_Sequential_GetUpdateFactor ( model ) ) ;
end
