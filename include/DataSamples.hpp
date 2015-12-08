#ifndef DATA_SAMPLES_H
#define DATA_SAMPLES_H

#include <stdint.h>
#include <atomic>

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

template <class T, uint32_t NumBuf = 3>
class RwBuffer
{
public:
	// This class is neither thread-safe nor necessarily correct (due to potentially incorrect use of atomics)
	// However it is used in interrupt code so mutexes are not really an option
	RwBuffer()
		: m_readIdx(0)
	{}

	T const& GetReadBuf() const
	{
		return m_buffer[m_readIdx];
	}

	T& GetWriteBuf()
	{
		unsigned int const writeIdx = (m_readIdx + 1) % NumBuf;
		return m_buffer[writeIdx]; // TODO verify if this actually works
	}

	void Flip()
	{
		uint32_t readIdx = m_readIdx;
		readIdx = (readIdx + 1) % NumBuf;
		// Atomically flip the readIdx
		m_readIdx.store(readIdx);
	}

private:
	std::atomic<unsigned int> m_readIdx;
	T m_buffer[NumBuf];

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
	: m_bHasBeenReset(false)
  {}

  void Reset()
  {
	m_sampleDataBuf.Flip();
	SampleData& sampleData = m_sampleDataBuf.GetWriteBuf();
    // we don't care about startMillis - we will just reset it when adding
    // the first sample
	sampleData.numMeasurements = 0;
	sampleData.numPwmPinHigh = 0;
	sampleData.numTtlPinHigh = 0;

	m_bHasBeenReset = true;
  }

  void DoSample(uint8_t pwmPin, uint8_t ttlPin)
  {
	SampleData& sampleData = m_sampleDataBuf.GetWriteBuf();
    if (sampleData.numMeasurements == 0)
    {
      // This is our first measurement
    	sampleData.startMillis = millis();
    }
    int pwmVal = digitalRead(pwmPin);
    int ttlVal = digitalRead(ttlPin);
    sampleData.numPwmPinHigh += (pwmVal) ? 1 : 0;
    sampleData.numTtlPinHigh += (ttlVal) ? 1 : 0;
    sampleData.numMeasurements++;
  }

  SampleData GetData() const
  {
	if (!m_bHasBeenReset)
		return SampleData();
    return m_sampleDataBuf.GetReadBuf();
  }

private:
  RwBuffer<SampleData> m_sampleDataBuf;
  bool m_bHasBeenReset;
};

#endif
