## E-book reader for Sailfish OS

![icon](app/harbour-books.png)

The core functionality is based on [FBReader](https://github.com/geometer/FBReader)
source code with a few modifications. Books are imported from the Downloads folder,
where they are saved by the browser or email client. Alternatively, you can
manually copy your books to the `Documents/Books` directory under the home
folder. Removable storage is supported as well, the books are stored in the
`/Books` directory there.

In theory, it should be able to handle all E-book formats supported by FBReader.
Tested mostly with epub and fb2.
