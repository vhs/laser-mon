#ifndef DATA_SAMPLES_H
#define DATA_SAMPLES_H

#include <stdint.h>

/**
 * Holds the data during a sampling period
 */
struct SampleData
{
  SampleData()
    : startMillis(0)
    , numMeasurements(0)
    , numPwmPinHigh(0)
    , numTtlPinHigh(0)
  {}
  // Start time of the measurements
  uint32_t startMillis;
  // Number of measurements taken
  uint32_t numMeasurements;
  // Number of times that the PWM pin was high when sampled
  uint32_t numPwmPinHigh;
  // Number of times that the TTL pin was high when sampled
  uint32_t numTtlPinHigh;
};

/**
 * Handles the state of SampleData
 * This will sample the PWM and TTL pins and accumulate their results in SampleData
 * The class will also make sure that the state of the struct is consistent
 * when resetting.
 */
class DataSampler
{
public:
  DataSampler()
  {}

  void Reset()
  {
    // we don't care about startMillis - we will just reset it when adding
    // the first sample
    m_sampleData.numMeasurements = 0;
    m_sampleData.numPwmPinHigh = 0;
    m_sampleData.numTtlPinHigh = 0;
  }

  void DoSample(uint8_t pwmPin, uint8_t ttlPin)
  {
    if (m_sampleData.numMeasurements == 0)
    {
      // This is our first measurement
      m_sampleData.startMillis = millis();
    }
    int pwmVal = digitalRead(pwmPin);
    int ttlVal = digitalRead(ttlPin);
    m_sampleData.numPwmPinHigh += (pwmVal) ? 1 : 0;
    m_sampleData.numTtlPinHigh += (ttlVal) ? 1 : 0;
    m_sampleData.numMeasurements++;
  }

  SampleData GetData() const
  {
    return m_sampleData;
  }

private:
  SampleData m_sampleData;
};

#endif
