clear all

receiver = OBNNode('receiver', 'obnep', 'mqtt');    % The OBN node
receiver.createInputPort('u', 'v', 'double');   % Input from E+
receiver.createOutputPort('y', 'v', 'double');  % Output to E+

global INPUTS;
INPUTS = [];

receiver.addCallback(@save_input_callback, 'Y', 0, receiver);
receiver.runSimulation(20)