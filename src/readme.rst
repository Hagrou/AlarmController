Compilation
===========


Pour "simplifier" la compilation et propager des valeurs identiques dans les scripts domoticz et dans le senseur alarmController, il est conseillé d'utiliser le petit programme "genConfig.py"

GenConfig
---------

Le programme genConfig.py prend un fichier de configuration et en propage les valeurs dans les différents code. Un répertoire cible est produit "build" ; c'est ce répertoire qui va contenir les sources à compiler et à déployer.

Pour fonctionner, genConfig à besoin de pyyaml que vous pouvez installer avec::

    $ pip install pyyaml

Voici une commande typique de configuration du source :

::

    $ python genConfig.py --conf=config_prod.yaml --src_dir=. --build_dir=./target

    avec :
        - conf : utilisation de la configuration de production
        -  src_dir : origine des sources
        -  build_dir : repertoire cible


Si tout ce passe bien, on obtient le log suivant :

::

    srcDir=[.]  configFile=[config_prod.yaml] buildDir=[./target]
    build_dir already exists, clean it
    Copy ./alarmController/.vscode/arduino.json -> ./target/alarmController/.vscode/arduino.json
    Copy ./alarmController/.vscode/c_cpp_properties.json -> ./target/alarmController/.vscode/   c_cpp_properties.json
    Copy ./alarmController/.vscode/settings.json -> ./target/alarmController/.vscode/settings.json
    Copy ./alarmController/alarmController.ino -> ./target/alarmController/alarmController.ino
    Copy ./alarmController/Buzzer.cpp -> ./target/alarmController/Buzzer.cpp
    Copy ./alarmController/Buzzer.h -> ./target/alarmController/Buzzer.h
    Patch File ./target/alarmController/config.h
    Copy ./alarmController/Vigil.cpp -> ./target/alarmController/Vigil.cpp
    Copy ./alarmController/Vigil.h -> ./target/alarmController/Vigil.h
    Copy ./alarmController/Wiegand.cpp -> ./target/alarmController/Wiegand.cpp
    Copy ./alarmController/Wiegand.h -> ./target/alarmController/Wiegand.h
    Copy ./alarmController/Wiegand_README.md -> ./target/alarmController/Wiegand_README.md
    Patch File ./target/domoticz/script_device_security2Warning.lua
    Patch File ./target/domoticz/script_time_securityStatus.lua


On trouve ainsi dans les fichiers suivants dans le répertoire target:

::

    target
    ├── alarmController
    │   ├── alarmController.ino
    │   ├── Buzzer.cpp
    │   ├── Buzzer.h
    │   ├── config.h
    │   ├── Vigil.cpp
    │   ├── Vigil.h
    │   ├── Wiegand.cpp
    │   ├── Wiegand.h
    │   └── Wiegand_README.md
    └── domoticz
        ├── script_device_security2Warning.lua
        └── script_time_securityStatus.lua

Compilation de target/alarmController
-------------------------------------

Nous allons maintenant compiler la partie alarmController qui va être ensuite uploadée sur l'esp8266. Après avoir configurer l'IDE Arduino pour programmer l'esp8266, ajouter dans l'environnement la librairie elapsedMillis (de Paul Stoffregen), puis chargez le sketch target/alarmController/alarmController.ino

.. Note:: normalement l'ensemble des valeurs configurées sont présentes dans le fichier config.h

.. Note:: penser a configurer domoticz pour accepter les requetes de alarmController (Setup->Settings->Local Networks

Configuration de l'IDE arduino:

Board: 
  - ESP8266 Community v2.7.1
  - NodeMCU 1.0 ESP 12E Module

Flash Size: 
  - 4MB, FS: 1MB OTa:~1019KB


Mise à jour Other The Air:
--------------------------

La version 2.0 permet la mise à jour via le Wifi en utilisant le mécanisme OTA offert sur l'ESP.

Pour l'utiliser il faut :

1. trouver le nom mDNS du sensor avec la commande avahi-browse 

    $ avahi-browse -t _arduino._tcp
    + enp0s3 IPv4 esp8266-alarmController                       _arduino._tcp        local


2. ensuite utiliser le port "Network" de l'IDE Arduino avec cet identifiant.










