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

function [ model ] = libvibeModel_Sequential_New ()
    
    % Parameters
	model.width                   = 0 ;
	model.height                  = 0 ;
	model.channels                = 0 ;
    
    model.numberOfSamples         = 20 ;
    model.matchingThreshold       = 20 ;
    model.matchingNumber          = 2 ;
    model.updateFactor            = 16 ;
    
    % Storage for the history
    model.historyBuffer           = [] ;

    % Buffers with random values
    model.update                  = [] ;
    model.neighbor_row            = [] ;
    model.neighbor_col            = [] ;
    model.position                = [] ;

    % Some other precomputations
    model.row                     = [] ;
    model.col                     = [] ;

end
