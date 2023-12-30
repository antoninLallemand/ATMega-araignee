#include "CircularBufferAL.h"


void CircularBufferAL :: begin(uint16_t bufferSize){
    CircularBufferSize = bufferSize;    //set buffer size
}

void CircularBufferAL :: writeData(int32_t data) {
    circularBuffer[writeIndex] = data;  //add an element
    if(writeIndex == CircularBufferSize-1) IndexScale = true; //when writing cursor come back to 0
    writeIndex = (writeIndex + 1) % CircularBufferSize; //increment write cursor for next data
    if(IndexScale && writeIndex > readIndex) IndexScale = false; //when writing cursor overtakes reading cursor
}

int32_t CircularBufferAL :: readData() {
    int32_t data = circularBuffer[readIndex];   //read oldest unread element
    if(readIndex == CircularBufferSize-1) IndexScale = false; //when reading cursor come back to 0
    if(readIndex < writeIndex || IndexScale) //increment only if buffer is'nt empty
        readIndex = (readIndex + 1) % CircularBufferSize;   //increment read cursor for next reading
    return data;
}

bool CircularBufferAL :: isEmpty(){
    return readIndex >= writeIndex && !IndexScale; //if reading and writing cursor are equal in the same round
}

