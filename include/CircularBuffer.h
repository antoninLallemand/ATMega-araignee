// Library managing a circular buffer
// Contain 4 public functions
// Author : Antonin Lallemand (2023)

#ifndef CircularBuffer_h
#define CircularBuffer_h

const int bufferSize = 100;

class CircularBuffer {
public :

    /*!
    * \brief function to be implemented in the setup loop of the main code
    * \param bufferSize maximum number of elements in the buffer
    */
    void begin(uint16_t bufferSize);

    /*!
    * \brief adding an element in the buffer
    * \param data data to add in the buffer
    */
    void writeData(int32_t data);

    /*!
    * \brief return the oldest unread value
    */
    int readData();

    /*!
    * \brief return true if the buffer is empty
    */
    bool isEmpty();

private :
  int32_t circularBuffer[bufferSize];
  uint16_t writeIndex = 0;
  uint16_t readIndex = 0;
  uint16_t CircularBufferSize = 100; //default value
  bool IndexScale = false; //true if writeIndex has one round advance on readIndex
};

#endif