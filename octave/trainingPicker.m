% create figure and panel on it
f = figure;
p = uipanel ("title", "What is this?", "position", [.7 0 .5 1]);

% add two buttons to the panel
b1 = uicontrol ("parent", p, "string", "Whistle", "position",[18 150 140 36]);
b2 = uicontrol ("parent", p, "string", "Echolocation", "position",[18 110 140 36]);
b3 = uicontrol ("parent", p, "string", "Background", "position",[18 60 140 36]);
b4 = uicontrol ("parent", p, "string", "Skip", "position",[18 10 140 36]);
