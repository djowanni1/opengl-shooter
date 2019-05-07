from skimage.io import imread, imsave
import numpy as np
def kek(file, x, y, k=0):
    mda = imread(file)
    imsave(file.replace('tga', 'jpg'), np.rot90(mda[::y, ::x, :], k))

kek('front.tga', -1, 1)
kek('back.tga', -1, 1)
kek('top.tga', -1, 1, 1)
kek('bottom.tga', -1, 1, -1)
kek('left.tga', -1, 1)
kek('right.tga', -1, 1)
