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

function [ model ] = libvibeModel_Sequential_Update ( model , image , updating_mask )
	
% TODO: check that the model has been initialized

    assert ( isa ( image , 'uint8' ) )
    assert ( size ( image , 1 ) == model.height )
    assert ( size ( image , 2 ) == model.width )
    assert ( size ( image , 3 ) == model.channels )
	assert ( and ( model.width > 0 , model.height > 0 ) )
	
	width = model.width ;
	height = model.height ;
    
    % Perturbate the precompted random arrays
    amount = width * height ;
    random_integers = randi ( [ 1 , amount ] , 1 , 1 ) ;
    r = random_integers ( 1 ) ;
    model.update = [ ( model.update ( r + 1 : end ) ) , ( model.update ( 1 : r ) ) ] ;
    amount = numel ( model.position ) ;
    random_integers = randi ( [ 1 , amount ] , 1 , 3 ) ;
    r = random_integers ( 1 ) ;
    model.neighbor_row = [ ( model.neighbor_row ( r + 1 : end ) ) , ( model.neighbor_row ( 1 : r ) ) ] ;
    r = random_integers ( 2 ) ;
    model.neighbor_col = [ ( model.neighbor_col ( r + 1 : end ) ) , ( model.neighbor_col ( 1 : r ) ) ] ;
    r = random_integers ( 3 ) ;
    model.position = [ ( model.position ( r + 1 : end ) ) , ( model.position ( 1 : r ) ) ] ;
    
    % Restrict the zone that will be updated
    update = and ( model.update , updating_mask ( : ) .' ) ;
    num_updates = sum ( update ) ;
    
    if model.channels == 1
        
        % Code optimization: in the calls to sub2ind hereafter, we ignore
        % the channel dimension. Specifying a fourth dimension of size 1,
        % with all indices being 1 does not change anything.

        % Replace one value of the history, at the pixel, by the current value
        row = model.row ( update ) ;
        col = model.col ( update ) ;
        pos = model.position ( 1 : num_updates ) ;
        ind = sub2ind ( [ height width model.numberOfSamples ] , row , col , pos ) ;
        val = image ( update ) ;
        model.historyBuffer ( ind ) = val ;

        % Replace one value of the history, at a neighbor pixel, by the current value
        row = row + model.neighbor_row ( 1 : num_updates ) ;
        row = max ( min ( row , height ) , 1 ) ;
        col = col + model.neighbor_col ( 1 : num_updates ) ;
        col = max ( min ( col , width ) , 1 ) ;
        ind = sub2ind ( [ height width model.numberOfSamples ] , row , col , pos ) ;
        model.historyBuffer ( ind ) = val ;
        
    elseif model.channels == 3
        
        image_r = image ( : , : , 1 ) ;
        image_g = image ( : , : , 2 ) ;
        image_b = image ( : , : , 3 ) ;
        
        val_r = image_r ( update ) ;
        val_g = image_g ( update ) ;
        val_b = image_b ( update ) ;
        
        % Replace one value of the history, at the pixel, by the current value
        row = model.row ( update ) ;
        col = model.col ( update ) ;
        pos = model.position ( 1 : num_updates ) ;
        ind = sub2ind ( [ height width 3 model.numberOfSamples ] , row , col , ones ( size ( pos ) ) , pos ) ;
        model.historyBuffer ( ind ) = val_r ;
        ind = ind + width * height ;
        model.historyBuffer ( ind ) = val_g ;
        ind = ind + width * height ;
        model.historyBuffer ( ind ) = val_b ;

        % Replace one value of the history, at a neighbor pixel, by the current value
        row = row + model.neighbor_row ( 1 : num_updates ) ;
        row = max ( min ( row , height ) , 1 ) ;
        col = col + model.neighbor_col ( 1 : num_updates ) ;
        col = max ( min ( col , width ) , 1 ) ;
        ind = sub2ind ( [ height width 3 model.numberOfSamples ] , row , col , 0 * pos + 1 , pos ) ;
        model.historyBuffer ( ind ) = val_r ;
        ind = ind + width * height ;
        model.historyBuffer ( ind ) = val_g ;
        ind = ind + width * height ;
        model.historyBuffer ( ind ) = val_b ;
        
    else
        
        assert ( false ) ;
    
    end
    
end
