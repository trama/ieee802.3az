
clc;
clear all;
close all;
% adding libraries from the system, to avoid broken version number among those provided
% by Matlab and those  provided by the system
%setenv('LD_LIBRARY_PATH', ['/lib64:' 'LD_LIBRARY_PATH'])
load cfr_100M.mat;
setenv('PATH', [getenv('PATH') ':/usr/local/bin:/usr/texbin'])

h1 = figure(1);
subplot1 = subplot(4,1,1,'Parent',h1,...
    'XTickLabel',['0 ';'1 ';'2 ';'3 ';'4 ';'5 ';'6 ';'7 ';'8 ';'9 ';'10';'11';'12'],...
    'XTick',[0 0.001 0.002 0.003 0.004 0.005 0.006 0.007 0.008 0.009 0.01 0.011 0.012]);
box(subplot1,'on');
plot( t_noeee(352:562) - t_noeee(353) , noeee(352:562), 'r-','LineWidth',1, 'Parent', subplot1 );
ylabel('Power [W]');
set(gca,'xtick', 0:0.001:0.012 , 'xticklabel',{})
ylim([0 30000]);
set(gca,'ytick', 0:5000:30000, 'yticklabel', ['0 ';'5 ';'10';'15';'20';'25';'  '] )
set(gca,'YGrid','on')
legend('No EEE')

subplot2 = subplot(4,1,2,'Parent',h1);
box(subplot2,'on');
plot( t_eee2_intel(16716:32776) - t_eee2_intel(16717), eee2_intel(16716:32776), 'Parent', subplot2,...
    'MarkerSize',3,'Marker','x','Color',[0 122 184]/256,'LineWidth',0.5);
ylim([0 30000]);
ylabel('Power [W]');
set(gca,'xtick', 0:0.001:0.012 , 'xticklabel',{})
set(gca,'ytick', 0:5000:30000 , 'yticklabel',['0 ';'5 ';'10';'15';'20';'25';'  '] )
set(gca,'YGrid','on')
legend('EEE-0')

subplot3 = subplot(4,1,3,'Parent',h1);
box(subplot3,'on');
plot( t_eee1(484:826) - t_eee1(485) , our_eee1(484:826), 'k-.','LineWidth',1, 'Parent', subplot3 );
ylabel('Power [W]');
ylim([0 30000]);
set(gca,'ytick', 0:5000:30000 , 'yticklabel',['0 ';'5 ';'10';'15';'20';'25';'  '] )
set(gca,'xtick', 0:0.001:0.012 , 'xticklabel',{})
set(gca,'YGrid','on')
legend('EEE-1')

subplot4 = subplot(4,1,4,'Parent',h1);
box(subplot4,'on');
plot( t_eee2(612:1082) - t_eee2(613), our_eee2(612:1082), 'k-','LineWidth',0.5, 'Parent', subplot4);
ylabel('Power [W]');
ylim([0 30000]);
set(gca,'ytick', 0:5000:30000 , 'yticklabel',['0 ';'5 ';'10';'15';'20';'25';'  '] )
set(gca,'xtick', 0:0.001:0.012 , 'xticklabel',{})
set(gca,'YGrid','on')
legend('EEE-2')
xlabel('Time [ms]');
samexaxis('abc','xmt','on','ytac','join','yld',1)
axis([-0.0002 0.012 0 30000])
set(gca,'XTick', 0:0.001:0.012)
set(gca,'XTickLabel',char('0','1','2','3','4','5','6','7','8','9','10','11','12'));
%legend('EEE disabled', 'EEE-1','EEE-2', 'EEE-2 (Intel)', 'Location','NorthEast')
%title('Instantaneous power consumption (100BASE-TX)')
adjust_figure(h1, 10,1)
nome = 'cfr_100M';
print('-depsc2','-loose', [nome '.eps'])
unix(['ps2pdf ' nome '.eps' ]);
unix(['pdfcrop ' nome '.pdf ' nome '.pdf']);


ts_noeee = timeseries(noeee, t_noeee);
m_noeee = mean(ts_noeee, 'Weighting', 'time')

ts_eee1 = timeseries(our_eee1, t_eee1);
m_eee1 = mean(ts_eee1, 'Weighting', 'time')

ts_eee2 = timeseries(our_eee2, t_eee2);
m_eee2 = mean(ts_eee2, 'Weighting', 'time')

ts_eee2intel = timeseries(eee2_intel, t_eee2_intel);
m_eee2intel = mean(ts_eee2intel, 'Weighting', 'time')
