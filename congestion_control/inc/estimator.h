#ifndef _RTC_ESTIMATOR_H
#define _RTC_ESTIMATOR_H

#include <iostream>
#include <deque>
#include "absl/strings/match.h"
#include "absl/types/optional.h"

#define RTC_CHECK(condition)           \
  if(!condition)                       \
              std::cout << __FILE__ << "," << __LINE__ <<": "<< #condition << std::endl

#define RTC_DCHECK(condition) RTC_CHECK(condition)
class RtcEstimator {
public:
    void Init();
};

struct TrendlineEstimatorSettings {
  static constexpr char kKey[] = "WebRTC-Bwe-TrendlineEstimatorSettings";
  static constexpr unsigned kDefaultTrendlineWindowSize = 20;

  //TrendlineEstimatorSettings() = delete;
  //explicit TrendlineEstimatorSettings(
  //    const WebRtcKeyValueConfig* key_value_config);

  // Sort the packets in the window. Should be redundant,
  // but then almost no cost.
  bool enable_sort = false;

  // Cap the trendline slope based on the minimum delay seen
  // in the beginning_packets and end_packets respectively.
  bool enable_cap = false;
  unsigned beginning_packets = 7;
  unsigned end_packets = 7;
  double cap_uncertainty = 0.0;

  // Size (in packets) of the window.
  unsigned window_size = kDefaultTrendlineWindowSize;

  //std::unique_ptr<StructParametersParser> Parser();
};

struct PacketTiming {
PacketTiming(double arrival_time_ms,
                double smoothed_delay_ms,
                double raw_delay_ms)
    : arrival_time_ms(arrival_time_ms),
        smoothed_delay_ms(smoothed_delay_ms),
        raw_delay_ms(raw_delay_ms) {}
double arrival_time_ms;
double smoothed_delay_ms;
double raw_delay_ms;
};


absl::optional<double> LinearFitSlope(
    const std::deque<PacketTiming>& packets) {
  RTC_DCHECK(packets.size() >= 2);
  // Compute the "center of mass".
  double sum_x = 0;
  double sum_y = 0;
  for (const auto& packet : packets) {
    sum_x += packet.arrival_time_ms;
    sum_y += packet.smoothed_delay_ms;
  }
  double x_avg = sum_x / packets.size();
  double y_avg = sum_y / packets.size();
  // Compute the slope k = \sum (x_i-x_avg)(y_i-y_avg) / \sum (x_i-x_avg)^2
  double numerator = 0;
  double denominator = 0;
  for (const auto& packet : packets) {
    double x = packet.arrival_time_ms;
    double y = packet.smoothed_delay_ms;
    numerator += (x - x_avg) * (y - y_avg);
    denominator += (x - x_avg) * (x - x_avg);
  }
  if (denominator == 0)
    return absl::nullopt;
  return numerator / denominator;
}

absl::optional<double> ComputeSlopeCap(
    const std::deque<PacketTiming>& packets,
    const TrendlineEstimatorSettings& settings) {
  RTC_DCHECK(1 <= settings.beginning_packets &&
             settings.beginning_packets < packets.size());
  RTC_DCHECK(1 <= settings.end_packets &&
             settings.end_packets < packets.size());
  RTC_DCHECK(settings.beginning_packets + settings.end_packets <=
             packets.size());
  PacketTiming early = packets[0];
  for (size_t i = 1; i < settings.beginning_packets; ++i) {
    if (packets[i].raw_delay_ms < early.raw_delay_ms)
      early = packets[i];
  }
  size_t late_start = packets.size() - settings.end_packets;
  PacketTiming late = packets[late_start];
  for (size_t i = late_start + 1; i < packets.size(); ++i) {
    if (packets[i].raw_delay_ms < late.raw_delay_ms)
      late = packets[i];
  }
  if (late.arrival_time_ms - early.arrival_time_ms < 1) {
    return absl::nullopt;
  }
  return (late.raw_delay_ms - early.raw_delay_ms) /
             (late.arrival_time_ms - early.arrival_time_ms) +
         settings.cap_uncertainty;
}

#endif