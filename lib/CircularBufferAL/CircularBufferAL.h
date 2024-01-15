// Library managing a circular buffer
// Contain 4 public functions
// Author : Antonin Lallemand (2023)

#ifndef CircularBufferAL_h
#define CircularBufferAL_h

#include <stdint.h>

const int bufferSize = 100;

class CircularBufferAL {
public :

    /*!
    * \brief adding an element in the buffer
    * \param data data to add in the buffer
    */
    void writeData(int32_t data);

    /*!
    * \brief return the oldest unread value
    */
    int32_t readData();

    /*!
    * \brief return true if the buffer is empty
    */
    bool isEmpty();

private :
  static const uint16_t CircularBufferSize = 100; //default value
  int32_t circularBuffer[CircularBufferSize];
  uint16_t writeIndex = 0;
  uint16_t readIndex = 0;
};

#endif