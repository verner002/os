# Kernel Object
`struct __kobj` predstavuje spolecne rozhrani pro libovolny prostredek systemu, u ktereho ma smysl cist nebo ukladat nejake parametry (sbernice, zarizeni, ovladac,...), struktura je nasledujici:

```c
struct __kobj {
    //atomic_t krefs;
    struct __sysfs_ops *kops;
    struct __sysfs_attrib **kattribs;
};
```

Jedna se o podobny typ jako v Linuxu, ovsem neobsahuje `kobj_type` a nedeli se do skupin pomoci struktury typu `kset`.

`struct __sysfs_ops` popisuje standardni rozhrani pro praci s atributy (konkretne funkce pro precteni a zapsani hodnoty) a `struct __sysfs_attrib **kattribs` je ukazatel na pole s atributy.

```c
struct __sysfs_attrib {
    char const *name;
    uint32_t owner;
    uint32_t mode;
};
```

```c
struct __sysfs_ops {
    int32_t (*read)(struct __kobj *kobj, char const *name, char *buffer);
    int32_t (*write)(struct __kobj *kobj, char const *name, char const *buffer, uint32_t count);
}
```

# Device & Driver Model
Zakladnim prepokladem pro pouzivani zarizeni je jejich detekce. O to by se mel starat ovladac sbernice, ktery detekuje pritomnost vsech pripojenych zarizeni. Inicializace techto zarizeni bude prenechana ovladacum danych zarizeni. Tedy ovladac PCI sbernice detekuje vsechna pritomna zarizeni a prida je do seznamu. Nasledne dojde k nacteni ovladacu zarizeni a ty se postaraji o inicializaci zarizeni. Tyto ovladace mohou vyuzivat funkci ovladace sbernice (registrace driveru, odpojeni zarizeni apod.).

Samotny ovladac sbernice muze byt take registrovan jako zarizeni, ktere vsak bude mit jako korenovou sbernici "system bus" (obecne nejaky koren). Zpravidla je totiz rizena nejakym radicem a ma tedy smysl ji vnimat jako zarizeni.

Na druhou stranu rozhrani pozadovane po sbernicich se od rozhrani zarizeni lisi a struktura reprezentujici zarizeni by mohla byt bud nedostatecna nebo naopak prehlcena. 

Kazdy ovladac sbernice by mel poskytovat nasledujici rozhrani:

- `release` - uvolneni prostredku sbernice,
- `dev_register` - registrace zarizeni,
- `dev_flush` - invalidace cache zarizeni,
- `dev_release` - odpojeni zarizeni,
- `ioctl` - v/v funkce sbernice,
<!-- - `__bus_dev_release` - uvolneni registrovaneho zarizeni z pameti. -->

Toto rozhrani bude implicitne soucasti struktury popisujici sbernici:

```c
struct __bus {
    char const *name;
    struct __kobj kobj;
    int32_t (*release)(void);
    int32_t (*ioctl)(uint8_t cmd, void const *data);
    int32_t (*dev_register)(struct __dev *dev);
    int32_t (*dev_flush)(__kdev_t kdev);
    int32_t (*dev_release)(__kdev_t kdev);
};
```

```c
struct __device {
    __kdev_t kdev;
    struct __kobj kobj;
    struct __device *parent;
    struct __driver *driver;
    void *firmware_info;
    //int32_t (*release)(void);
    //int32_t (*ioctl)(uint8_t cmd, void const *data);
}
```

`kdev` je major:minor cislo zarizeni a urcuje skupinu a konkretni zarizeni z teto skupiny. `kobj` odkazuje na kernel objekt s atributy daneho zarizeni. `driver` referuje na ovladac, ktery si zarizeni asocioval. `firmware_info` odkazuje na data poskytnuta firmwarem zarizeni.

```c
struct __char_device {
    struct __device h;
    // count represents a char count
    int32_t (*read)(uint32_t offset, uint32_t count, char *buffer);
    int32_t (*write)(uint32_t offset, uint32_t count, char const *buffer);
    int32_t (*ioctl)(uint8_t cmd, void *data);
};
```

```c
// no buffering
struct __block_device {
    struct __device h;
    // count represents a block count
    int32_t (*read)(uint32_t offset, uint32_t count, char *buffer);
    int32_t (*write)(uint32_t offset, uint32_t count, char const *buffer);
    // for block device with char-access support
    int32_t (*ioctl)(uint8_t cmd, void *data);
};
```

```c
struct __driver {
    char const *name;
    void *firmware_info;
}
```

Informace o sbernicich a zarizenich je mozne najit ve virtualni souborovem systemu v `/sys`.

Ovladac zarizeni muze existovat bez zarizeni. Zarizeni muze existovat bez ovladace, avsak nebude mozne jej namountovat a provadet s nim zakladni operace.