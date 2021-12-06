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

function [ model ] = libvibeModel_Sequential_AllocInit ( model , image , neighborhood_radius )
    
    assert ( isa ( image , 'uint8' ) , 'wrong usage' )
    assert ( neighborhood_radius >= 0 , 'wrong usage' )

    width = size ( image , 2 ) ;
    height = size ( image , 1 ) ;
    channels = size ( image , 3 ) ;

    assert ( and ( width > 1 , height > 1 ) , 'wrong usage' )
    assert ( or ( channels == 1 , channels == 3 ) , 'wrong usage' )

    model.width = width ;
    model.height = height ;
    model.channels = channels ;
    
    model.row = repmat ( ( 1 : height )' , 1 , width ) ;
    model.col = repmat ( 1 : width , height , 1 ) ;

    model.historyBuffer = zeros ( height , width , channels , model.numberOfSamples , 'int16' ) ;
    
    % We want to guarantee at least two matches in each pixel just after
    % the initialization, in order to predict only background. So, the
    % random noise (this idea originates from the C implementation by
    % Marc Van Droogenbroeck) is only added on numberOfSamples-matchingNumber
    % samples.

    image = int16 ( image ) ;
    for test = 1 : model.matchingNumber
        model.historyBuffer ( : , : , : , test ) = image ;
    end
    for test = model.matchingNumber + 1 : model.numberOfSamples
        value_plus_noise = image + int16 ( randi ( [ -20 20 ] , height , width , channels ) ) ;
        value_plus_noise = max ( value_plus_noise , int16 ( 0 ) ) ;
        value_plus_noise = min ( value_plus_noise , int16 ( 255 ) ) ;
        model.historyBuffer ( : , : , : , test ) = value_plus_noise ;
    end
    
    % For speed reasons, we precompute arrays of random numbers.
    % The first one is an array of Booleans, used to know in which pixel
    % the model will be updated. The proportion of "true"s depends on the
    % update factor. Note that the following code guarantees that some
    % update will be done, even if the update factor is huge (but not
    % infinite, indeed). But this is a detail.

    amount = width * height ;
    model.update = false ( 1 , amount ) ;
    model.update ( 1 : ceil ( amount / model.updateFactor ) ) = true ;
    model.update = model.update ( randperm ( amount ) ) ;
    
    % The next precomputed arrays of random intergers specify the
    % displacement when a sample is added to the history model. Of course,
    % one array could be sufficient. I prefer to declare two in case one
    % day one would like to give different bounds for the horizontal and
    % vertical displacements. The length of the arrays does not need to
    % exceed the maximum number of pixels that will be considered to the
    % model update. Perhaps you be surpised that I do not something like:
    % randi ( [ -neighborhood_size neighborhood_size ] , 1 , amount ). Once
    % again, that's a small detail, but if I did that I would have no
    % guarantee that the mean is zero, and that would result in a bias in
    % the direction of the sample propagation. So I prefer something a
    % little bit more complicated.
    
    amount = sum ( model.update ) ;
    model.neighbor_row = floor ( linspace ( -neighborhood_radius , neighborhood_radius + ( 1 - eps ) , amount ) ) ;
    model.neighbor_row = model.neighbor_row ( randperm ( amount ) ) ;
    model.neighbor_col = floor ( linspace ( -neighborhood_radius , neighborhood_radius + ( 1 - eps ) , amount ) ) ;
    model.neighbor_col = model.neighbor_col ( randperm ( amount ) ) ;
    
    % And the last array of random intergers specify the place in the
    % history that will be overriden when updating the model.
    
    model.position = randi ( [ 1 model.numberOfSamples ] , 1 , amount ) ;

end
