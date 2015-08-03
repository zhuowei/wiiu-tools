## Tutorial: Downloading game updates for datamining with UWizard

It's possible to download most Wii U game updates in order to explore changes between versions of games. Note that only some updates can be downloaded, not the games themselves. Also, not all updates can be downloaded - but many can be.

1) Download and start [UWizard](https://gbatemp.net/threads/uwizard-all-in-one-wii-u-pc-program.386508/)

2) Go to http://wiiubrew.org/wiki/Title_database and look for the title ID of the game that you want to datamine

For example, Splatoon (JAP)'s title ID is `00050000-10162B00`

3) To download updates, change the last zero before the dash to an E.

For Splatoon, this gives `0005000E-10162B00`.

4) remove the dash, and put the resulting title ID into UWizard's NUS Downloader U tab. (For Splatoon, that's `0005000E10162B00`)

5) for the latest update, leave the Version field blank. If you want older updates, most updates have versions that are powers of 2; e.g. 16, 32, 64

6) Only for the first time: Go to the Settings tab and input the Wii U common key. (I can't tell you what it is, but you can go search for it online)

Once the key is imported, return to the NUS Downloader U tab.

7) check "Decrypt contents", then hit "Start Download".

[screenshot for Splatoon](http://i.imgur.com/T8pVbNo.png)

For some games, the update data may not have a common E-ticket, which means that they can only be downloaded by an actual Wii U console, and cannot be datamined by this method. (You'll see an error message about missing common ticket in the NUS Downloader output)

8) Once the download completes, go to the folder you extracted UWizard to, and there should be a "nus_content" folder. Go inside it, then go into the title ID of the update you downloaded, then the version. The contents of the update should be there (you should see a code, content, and meta folder).

[screenshot for Splatoon](http://i.imgur.com/jZ9UYCx.png)

In updates, only files that changed from the previous version are included.

9) Explore the update data. Most games store their data in the contents/ folder, in archives. UWizard can extract most archives: consult its manual to learn how to use it to extract and view files.

For example, Splatoon's update version 64 (1.3.0) has two files in the contents/ folder: Pack/Env.pack and Pack/Static.pack. To extract the .Pack files:

10) Go to the Archive Manager tab of UWizard, then choose "Extract SARC"

11) change the file type from "SARC archives" to "All Files", then browse to and choose the Static.pack

12) a folder should pop up with the extracted contents of the Static.pack.

Happy exploring, and have fun!