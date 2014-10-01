
clc;
clear all;
close all;
% adding libraries from the system, to avoid broken version number among those provided
% by Matlab and those  provided by the system
setenv('LD_LIBRARY_PATH', ['/lib64:' 'LD_LIBRARY_PATH'])
load cfr_100M.mat;


h1 = figure(1);
plot( time1(452:end)-time1(452), eee1(452:end),'k-.','LineWidth',1);
hold on
plot(time2(452:end)-time2(452), eee2_adj(452:end),'k-','LineWidth',1)
xlabel('Time [ms]');
ylabel('Power [mW]');
axis([0 0.0051 5000 50000])
set(gca,'XTick', 0:0.001:0.005)
set(gca,'XTickLabel',char('0','1','2','3','4','5'));
legend('EEE-1','EEE-2', 'Location','Best')
title('Instantaneous power consumption (100BASE-TX)')
adjust_figure(h1, 10,1)
nome = 'cfr_100M';
print('-depsc2','-loose', [nome '.eps'])
unix(['epstopdf ' nome '.eps' ]);
unix(['pdfcrop ' nome '.pdf ' nome '.pdf']);


clear all;
close all;
load cfr_eee2.mat;

h2 = figure(2);
plot(t_eee2_100M(452:end)-t_eee2_100M(452), eee2_100M(452:end),'k-.','LineWidth',1);
hold on
plot(t_eee2_10G(541:end)-t_eee2_10G(541), eee2_10G(541:end),'k-')
xlabel('Time [ms]');
ylabel('Power [mW]');
axis([0 0.0051 5000 250000])
set(gca,'XTick', 0:0.001:0.005)
set(gca,'XTickLabel',char('0','1','2','3','4','5'));
legend('100BASE-TX','10GBASE-T', 'Location','NorthEast')
title('Instantaneous power consumption (EEE-2)')
adjust_figure(h2, 10,1)
nome = 'cfr_eee2';
print('-depsc2','-loose', [nome '.eps'])
unix(['epstopdf ' nome '.eps' ]);
unix(['pdfcrop ' nome '.pdf ' nome '.pdf']);

%set(gca,'XTick', -100:10:120)

close all;

h3 = figure(3);
plot(t_eee1_10G(455:end)-t_eee1_10G(455), eee1_10G(455:end),'-.','Color',[0.8 0.8 0.8],'LineWidth',1);
hold on
plot(t_eee2_10G(455:end)-t_eee2_10G(455), eee2_10G(455:end),'k-')
xlabel('Time [ms]');
ylabel('Power [mW]');
axis([0 0.0051 50000 370000])
set(gca,'XTick', 0:0.001:0.005)
set(gca,'XTickLabel',char('0','1','2','3','4','5'));
legend('EEE-1','EEE-2', 'Location','Best')
title('Instantaneous power consumption (10GBASE-T)')
adjust_figure(h2, 10,1)
nome = 'cfr_10G';
print('-depsc2','-loose', [nome '.eps'])
unix(['epstopdf ' nome '.eps' ]);
unix(['pdfcrop ' nome '.pdf ' nome '.pdf']);