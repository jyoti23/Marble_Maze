clc;
clear all;
fontSize = 20;  % Font size for image captions.

vid = videoinput('winvideo',3,'YUY2_176x144');
preview(vid);
i = getsnapshot(vid);
imshow(i);
originalImage = imcrop (i,[17 21 123 119]);	/* depends on video captured*/
%imshow (I);
% Open the maze image file.
	%originalImage = imread(I);
	[rows cols numberOfColorBands] = size(originalImage);

	% Convert to monochrome for processing.
	if numberOfColorBands > 1
		% Convert to monochrome.
		redPlane = originalImage(:, :, 1);
		greenPlane = originalImage(:, :, 2);
		bluePlane = originalImage(:, :, 3);

		% Find the standard deviation of each color channel.
		redStdDev = std(single(redPlane(:)));
		greenStdDev = std(single(greenPlane(:)));
		blueStdDev = std(single(bluePlane(:)));

		% Take the color channel with the highest contrast.
		% Transfer it into a monochrome image.  This will be the one that we use.
		if redStdDev >= greenStdDev && redStdDev >= blueStdDev
			% Red has most contrast - use that channel.
			monoImage = single(redPlane);
		elseif greenStdDev >= redStdDev && greenStdDev >= blueStdDev
			% Green has most contrast - use that channel.
			monoImage = single(greenPlane);
		else
			% Blue has most contrast - use that channel.
			monoImage = single(bluePlane);
		end
	else
		monoImage = single(originalImage);
	end


	% Now we have a monochrome image that we can use to solve the maze.
	% Display the results of this step.
	close all;	% Close any prior windows that are open from a prior run.
	subplot(2, 2, 1);
	imshow(monoImage, []);
	title('Original Image', 'FontSize', fontSize);
	set(gcf, 'Position', get(0,'Screensize')); % Maximize figure.current figure

	% Scale image to 0-255.
	maxValue = max(max(monoImage));
	minValue = min(min(monoImage));
	monoImage = uint8(255 * (single(monoImage) - minValue) / (maxValue - minValue));
	% Threshold to get the walls.  This will also sharpen up blurry, fuzzy wall edges.
	thresholdValue = uint8((maxValue + minValue) / 2);
	binaryImage = 255 * (monoImage < thresholdValue);
	% Display the results of this step.
	subplot(2, 2, 2);
	imshow(binaryImage, []);
	title('Binary Image - The walls are white here, instead of black', 'FontSize', fontSize);

	% Label the image to identify discrete, separate walls.
	[labeledImage numberOfWalls] = bwlabel(binaryImage, 4);     % Label each blob so we can make measurements of it
	coloredLabels = label2rgb (labeledImage, 'hsv', 'k', 'shuffle'); % pseudo random color labels
	% Display the results of this step.
	subplot(2, 2, 3);
	imshow(coloredLabels); 
	caption = sprintf('Labeled image of the %d walls, each a different color', numberOfWalls);
	title(caption, 'FontSize', fontSize);
	%if numberOfWalls ~= 2
		%message = sprintf('This is not a "perfect maze" with just 2 walls.\nThis maze appears to have %d walls,\nso you may get unexpected results.', numberOfWalls);
		%uiwait(msgbox(message));
	%end

	% Take the first label.  This will be the first wall.
	% For a perfect maze (only two walls) you can take either wall, so let's take the first one.
	binaryImage2 = (labeledImage == 1);
	% Display the results of this step.
	subplot(2, 2, 4);
	imshow(binaryImage2, []);
	title('One of the walls', 'FontSize', fontSize);

	% Dilate the walls by a few pixels
	dilationAmount = 7; % Number of pixels to dilate and erode.
	% IMPORTANT NOTE: dilationAmount is a parameter that you may 
	% wish to experiment with, trying different integer values.
	dilatedImage = imdilate(binaryImage2, ones(dilationAmount));

	figure;  % Create another, new figure window.
	set(gcf, 'Position', get(0,'Screensize')); % Maximize figure.current figure
    
	% Display the results of this step.
	subplot(2, 2, 1);
	imshow(dilatedImage, []);
	title('Dilation of one wall', 'FontSize', fontSize);

	filledImage = imfill(dilatedImage, 'holes');
	% Display the results of this step.
	subplot(2, 2, 2);%2*2 matrix of small axes
	imshow(filledImage, []);
	title('Now filled to get rid of holes', 'FontSize', fontSize);

	% Erode by the same amount of pixels
	erodedImage = imerode(filledImage, ones(dilationAmount));
	% Display the results of this step.
	subplot(2, 2, 3);
	imshow(erodedImage, []);
	title('Eroded', 'FontSize', fontSize);

	% Set the eroded part to zero to find the difference.
	% (You could also subtract the eroded from the filled, dilated image.
	solution = filledImage;
	solution(erodedImage) = 0;
	% Display the results of this step.
	subplot(2, 2, 4);
	imshow(solution, []);
	title('The Difference = The Solution', 'FontSize', fontSize);

	% Put the solution in red on top of the original image
	if numberOfColorBands == 1
		% If we're monochrome, we need to make the color planes.
		% If we're color, we already have these from above.
		redPlane = monoImage;
		greenPlane = monoImage;
		bluePlane = monoImage;
	end
	redPlane(solution) = 255;
	greenPlane(solution) = 0;
	bluePlane(solution) = 0;
	solvedImage = cat(3, redPlane, greenPlane, bluePlane);%concatenate all input array along dim
	% Display the results of this step.
	figure;  % Create another, new figure window.
	imshow(solvedImage);
	set(gcf, 'Position', get(0,'Screensize')); % Maximize figure.
	title('Final Solution Over Original Image', 'FontSize', fontSize);
	


