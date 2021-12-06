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

function [] = libvibeModel_Sequential_Demonstrate ( display , postprocessing_median_filter_size , neighborhood_radius , grayscale )
    
    if nargin < 1
        display = true ;
    end
    if nargin < 2
        % set to 5 to do the same as the postprocessing used on CDnet.
        % set to 3 to reproduce the typical results shown by the authors of ViBe.
        % set to 1 to disable the postprocessing
        postprocessing_median_filter_size = 3 ; 
    end
    if nargin < 3
        neighborhood_radius = 1 ; % default vaule
    end
    if nargin < 4
        grayscale = false ;
    end

    model = libvibeModel_Sequential_New () ;

    video_sequence = VideoReader ( 'driveway-320x240.mp4' ) ;
    fprintf ( 'File name : %s\n' , video_sequence.Name ) ;
    fprintf ( 'Full path to video file : %s\n' , video_sequence.Path ) ;
    fprintf ( 'Video format : %s\n' , video_sequence.VideoFormat ) ;
    fprintf ( 'Length of file : %.3f\n' , video_sequence.Duration ) ;
    fprintf ( 'Bits per pixel of video data : %u\n' , video_sequence.BitsPerPixel ) ;
    fprintf ( 'Number of video frames per second : %.3f\n' , video_sequence.FrameRate ) ;
    fprintf ( 'Width of video frame : %u\n' , video_sequence.Width ) ;
    fprintf ( 'Height of video frame : %u\n' , video_sequence.Height ) ;
    
    if display
        
        figure ;
        
        black = zeros ( video_sequence.Height , video_sequence.Width , 'double' ) ;
        
        subplot ( 2 , 2 , 1 ) ;
        h_input = imshow ( black ) ;
        title ( 'input frame' ) ;
        
        subplot ( 2 , 2 , 2 ) ;
        h_matches = imshow ( black ) ;
        title ( 'number of matches' ) ;

        subplot ( 2 , 2 , 3 ) ;
        h_mask = imshow ( black ) ;
        title ( 'output of ViBe' ) ;
        
        subplot ( 2 , 2 , 4 ) ;
        h_output = imshow ( black ) ;
        if postprocessing_median_filter_size > 1
            title ( sprintf ( 'ViBe + median filter (%u)' , postprocessing_median_filter_size ) ) ;
        else
            title ( 'postprocessing disabled' ) ;
        end
        
        drawnow nocallbacks
        
    end
    
    frame_idx = 0 ;
    t_start = tic () ;
    t = [] ;
    
    while hasFrame ( video_sequence )
        
        % Compute the frame number and the FPS statistic.
        
        frame_idx = frame_idx + 1 ;
        if numel ( t ) >= 25
            t = t ( 2 : end ) ;
        end
        t = [ t , ( toc ( t_start ) ) ] ;
        fps = ( numel ( t ) - 1 ) / ( t ( end ) - t ( 1 ) ) ;
        fprintf ( 'reading and processing frame %u ... @ %.2f FPS\n' , frame_idx , fps ) ;
        
        % Get the next frame from the video sequence.
        
        frame = readFrame ( video_sequence ) ;
        if grayscale && ndims ( frame ) > 2
            frame = rgb2gray ( frame ) ;
        end
        
        % Apply ViBe. Here, I decided to both segment and update the model.
        % Note that updating the model is optional.
        
        if frame_idx == 1
            model = libvibeModel_Sequential_AllocInit ( model , frame , neighborhood_radius ) ;
            libvibeModel_Sequential_PrintParameters ( model ) ;
            [ segmentation_map , num_matches ] = libvibeModel_Sequential_Segmentation ( model , frame ) ;
        else
            [ segmentation_map , num_matches ] = libvibeModel_Sequential_Segmentation ( model , frame ) ;
            model = libvibeModel_Sequential_Update ( model , frame , not ( segmentation_map ) ) ;
        end
        
        % Display everything
        
        if display
            % In oder to display, we convert the integers to floating point
            % values between 0.0 and 1.0.
            h_input.CData = double ( frame ) / 255.0 ;
            h_matches.CData = double ( num_matches ) / double ( libvibeModel_Sequential_GetNumberOfSamples ( model ) ) ;
            h_mask.CData = segmentation_map ;
            if postprocessing_median_filter_size > 1
                segmentation_map = medfilt2 ( segmentation_map , [ postprocessing_median_filter_size postprocessing_median_filter_size ] ) ;
            end
            h_output.CData = segmentation_map ;
            drawnow nocallbacks
        end
        
    end
    
end
