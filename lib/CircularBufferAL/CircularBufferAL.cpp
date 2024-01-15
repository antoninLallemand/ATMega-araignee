#include "CircularBufferAL.h"


void CircularBufferAL :: writeData(int32_t data) {
    circularBuffer[writeIndex] = data;  //add an element
    writeIndex = (writeIndex + 1) % CircularBufferSize; //increment write cursor for next data
}

int32_t CircularBufferAL :: readData() {
    int32_t data = circularBuffer[readIndex];   //read oldest unread element
    readIndex = (readIndex + 1) % CircularBufferSize;   //increment read cursor for next reading
    return data;
}

bool CircularBufferAL :: isEmpty(){
    return readIndex == writeIndex; //if reading and writing cursor are equal
}