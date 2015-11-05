function save_input_callback(receiver)
global INPUTS;

u = receiver.input('u')';
INPUTS(end+1,:) = u;

% The control algorithm
TCRooLow = 22;  % Zone temperature is kept between TCRooLow & TCRooHi
TCRooHi = 26;
TOutLow = 22;  % Low level of outdoor temperature
TOutHi = 24;  % High level of outdoor temperature
ratio = (TCRooHi - TCRooLow)/(TOutHi - TOutLow);

% Get the time of the current day, in hours [0, 24)
dayTime = mod(receiver.currentSimTime('h'), 24);
if (dayTime >= 6) && (dayTime <= 18)
    % It is day time (6AM-6PM)
    
    % The Heating set-point: day -> 20, night -> 16
    % The cooling set-point is bounded by TCRooLow and TCRooHi
    
    SP = [20, max(TCRooLow, ...
        min(TCRooHi, TCRooLow + (u(1) - TOutLow)*ratio))];
else
    % The Heating set-point: day -> 20, night -> 16
    % The Cooling set-point: night -> 30
    SP = [16 30];
end
receiver.output('y', SP);

end

