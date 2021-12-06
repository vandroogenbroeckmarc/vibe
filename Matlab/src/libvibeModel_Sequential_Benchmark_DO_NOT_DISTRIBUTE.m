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

function [] = libvibeModel_Sequential_Benchmark_DO_NOT_DISTRIBUTE ()

    t = [] ;

    for display = 0 : 1
        for postprocessing_median_filter_size = [ 1 3 5 ]
            for neighborhood_radius = 1 : 3
                for grayscale = 0 : 1
                    t_start = tic () ;
                    libvibeModel_Sequential_Demonstrate ( display , postprocessing_median_filter_size , neighborhood_radius , grayscale ) ;
                    t_elapsed = toc ( t_start ) ;
                    close all ;
                    t = [ t t_elapsed ] ;
                end
            end
        end
    end

    for display = 0 : 1
        for postprocessing_median_filter_size = [ 1 3 5 ]
            for neighborhood_radius = 1 : 3
                for grayscale = 0 : 1
                    disp ( sprintf ( 'libvibeModel_Sequential_Demonstrate ( %u , %u , %u , %u ) -> %.2f s.' , display , postprocessing_median_filter_size , neighborhood_radius , grayscale , t ( 1 ) ) ) ;
                    t = t ( 2 : end ) ;
                end
            end
        end
    end
    
end
