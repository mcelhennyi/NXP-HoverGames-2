import numpy as np
import cv2
import time

print("Opening video capture....")

cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
print("Opened")

FRAME_RATE = 13

# Define the codec and create VideoWriter object
fourcc = cv2.VideoWriter_fourcc('M','J','P','G')
out = cv2.VideoWriter(
    'output.avi',
    fourcc,
    FRAME_RATE,
    (640,480)
)

print("Starting recording...")

while cap.isOpened():
    try:
        before = time.time()
        ret, frame = cap.read()
        if ret==True:
            frame = cv2.flip(frame,0)

            # write the flipped frame
            out.write(frame)

            # cv2.imshow('frame', frame)
            # if cv2.waitKey(1) & 0xFF == ord('q'):
            #     break
        else:
            print("Failed to capture frame!")
            break

        after = time.time()
        delay = after - before
        if delay >= 1.0 / FRAME_RATE:
            print("Frame blown: " + str(1.0/delay) + " Hz")
        else:
            # Sleep the diff
            time.sleep((1.0 / FRAME_RATE) - delay)

    except KeyboardInterrupt as e:
        print("Closing recording")
        break

# Release everything if job is finished
cap.release()
out.release()
print("Closed")
cv2.destroyAllWindows()