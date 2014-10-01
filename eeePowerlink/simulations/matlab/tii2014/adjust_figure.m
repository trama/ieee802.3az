%TO BE USED: AFTER figure(#); BEFORE plot(...);
%see: HELP - Changing a Figure's Settings
function adjust_figure(fig,font_size,notimes,traces) %figure handle,number of traces, fontsize

if nargin==1
    font_size = 12;
end

ax=get(fig,'CurrentAxes');

%Set Text properties, and BOX ON
if(notimes)
    set(ax,'fontunits', 'points','fontsize',font_size,'FontName','Helvetica','NextPlot','replacechildren','box','on');
    set(get(ax,'YLabel'),'fontsize',font_size,'FontName','Helvetica')
    set(get(ax,'XLabel'),'fontsize',font_size,'FontName','Helvetica')
    set(get(ax,'ZLabel'),'fontsize',font_size,'FontName','Helvetica')
    set(get(ax,'Title'),'fontsize',font_size,'FontName','Helvetica')    
else
    if(isunix)
        set(ax,'fontunits', 'points','fontsize',font_size,'FontName','Times','NextPlot','replacechildren','box','on');
        set(get(ax,'YLabel'),'fontsize',font_size,'FontName','Times')
        set(get(ax,'XLabel'),'fontsize',font_size,'FontName','Times')
        set(get(ax,'ZLabel'),'fontsize',font_size,'FontName','Times')
        set(get(ax,'Title'),'fontsize',font_size,'FontName','Times')
    else
        set(ax,'fontunits', 'points','fontsize',font_size,'FontName','Times New Roman','NextPlot','replacechildren','box','on');
        set(get(ax,'YLabel'),'fontsize',font_size,'FontName','Times New Roman')
        set(get(ax,'XLabel'),'fontsize',font_size,'FontName','Times New Roman')
        set(get(ax,'ZLabel'),'fontsize',font_size,'FontName','Times New Roman')
        set(get(ax,'Title'),'fontsize',font_size,'FontName','Times New Roman')
    end
end

set(fig, 'Color',[1 1 1],'PaperPositionMode', 'manual','PaperUnits', 'centimeters','PaperPosition', [0 0 14 21]);

if(nargin>3)
    %Grayscale Color sequence for PLOT command
    colorsequence = [];
    for i=1:traces
        colorsequence = [ colorsequence; [i i i] ];
    end
    colorsequence = colorsequence / (i*traces);
    set(ax,'ColorOrder',colorsequence);
end