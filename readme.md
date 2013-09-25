# RtmpClient for Windows Runtime (Mntone::Rtmp, Windows 8.1)

## License
This library and demoapp is “3-clause BSD license.”
You must check “[license.en-US.md](./license.en-US.md).”

## API reference for RtmpClient.
If you know these in detail, you check “[Document](./Document/)” folder.

## Alpha version
This library is debuged with Windows 8.1 Preview. In this version, I cannot use **TypedEventHandler&lt;TSender, TArgs&gt;**. Thus, I use **EventHandler&lt;TArgs&gt;** instead of **TypedEventHandler&lt;TSender, TArgs&gt;**.

## Problem??
In Windows 8.1 Preview, its version (?) cannot play H.264/AVC video correctly. I do not know Windows bug or this program bug.

## TODO!!
- Implements exit transaction.