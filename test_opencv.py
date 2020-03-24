import cv2

def main( ):
    cap = cv2.VideoCapture( 0 )
    if cap.isOpened( ):
        while True:
            ret, frame = cap.read( )
            if ret:
                cv2.imshow( 'figure', frame )
                cv2.waitKey( 10 )
    else:
        print( 'Could not open camera' )


if __name__ == '__main__':
    main()
