# Test Server

This is a test server for FTP clients. You'll see this folder if you log in with anonymous user. The folder is read-only; for testing STOR, use the account `ssast2021` with the previously given password instead. Contact WeChat:panda2134 for more details.

## Folder Structure

- `images`: Some ISO files are located in this folder. As they're relatively big, you can use them to test the reliability of your client. For your convenience, we have put all of the sha256 checksums in a file called `SHA256` in the directory.
- `perm_test`: Inside the folder are some files & folders that you cannot open due to lack of permission. They're used to test handling of permission errors.
- `utf8`: Used for testing UTF8 encoding support. Filenames there can contain non-ASCII characters. See `utf8/filenames.png` for their correctly rendered form.
- `files`: Some small text files and source code files. Try downloading if you like.

