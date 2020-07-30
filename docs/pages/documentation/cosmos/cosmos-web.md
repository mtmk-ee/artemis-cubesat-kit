---
title: COSMOS Web
permalink: /pages/documentation/cosmos/cosmos-web.html
layout: page

tags: [software]
keywords: software

---


## What is COSMOS Web?


## Starting COSMOS Web

For convenience you should make a shell script. Create a file called `cosmos-web.sh` in the home directory in Ubuntu,
and paste the following contents:

```bash
#!/bin/bash

sudo systemctl start mongod

~/cosmos/tools/agent_mongo --realm artemis &
cd ~/cosmos/source/tools/cosmos-web
npm start && fg
```

Save the file, and run the following command in a local terminal:

```bash
sudo chmod +x ~/cosmos-web.sh
```

Now you can start COSMOS Web by running this script:

```bash
~/cosmos-web.sh
```

### Start Agent Mongo

Run the following command i


## Further Reading
