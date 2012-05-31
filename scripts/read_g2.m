function histograms = read_g2(filename)
% This script reads a g2 of t2 data from the named file, following the 
% format output by histogram.
%
% The output is a 2d array of histograms, indexed by the channels 
% corresponding to the particular cross-correlation. The histograms are
% represented as a 2d array, with the first column indicating the centers
% of each bin and the second column the counts in those bins.
%
% For example, to plot a histogram for the cross-correlation (1,1):
% histograms = read_g2('blargh.pt2.g2')
% plot(histograms{1,1}(:,1), histograms{1,1}(:,2))
stream = fopen(filename, 'r');
raw_histogram = textscan(stream, '%d32,%d32,%f64,%f64,%d64\n');

src_channels = raw_histogram{1};
bins_left = raw_histogram{3};
bins_right = raw_histogram{4};
counts = raw_histogram{5};

n_channels = src_channels(end)+1;
n_bins = length(src_channels)/(n_channels^2);

histograms = cell(2,2);

for src_channel=1:n_channels
    for dst_channel=1:n_channels
        correlation_index = (src_channel-1)*n_channels + dst_channel;
        limits = ((correlation_index-1)*n_bins+1):...
            ((correlation_index)*n_bins);
        
        bin_centers = (bins_right(limits)+bins_left(limits))/2;
        my_counts = counts(limits);
        
        histograms{src_channel, dst_channel} = [bin_centers my_counts];
    end
end

fclose(stream);

end