# Edit this configuration file to define what should be installed on
# your system. Help is available in the configuration.nix(5) man page, on
# https://search.nixos.org/options and in the NixOS manual (`nixos-help`).

{ config, lib, pkgs, ... }:

{
  imports =
    [ # Include the results of the hardware scan.
      ./hardware-configuration.nix
    ];

  # Use the GRUB 2 boot loader.
  boot.loader.grub.enable = true;
  # boot.loader.grub.efiSupport = true;
  # boot.loader.grub.efiInstallAsRemovable = true;
  # boot.loader.efi.efiSysMountPoint = "/boot/efi";
  # Define on which hard drive you want to install Grub.
  # boot.loader.grub.device = "/dev/sda"; # or "nodev" for efi only

  # networking.hostName = "nixos"; # Define your hostname.
  # Pick only one of the below networking options.
  # networking.wireless.enable = true;  # Enables wireless support via wpa_supplicant.
  # networking.networkmanager.enable = true;  # Easiest to use and most distros use this by default.

  # Set your time zone.
  # time.timeZone = "Europe/Amsterdam";

  # Configure network proxy if necessary
  # networking.proxy.default = "http://user:password@proxy:port/";
  # networking.proxy.noProxy = "127.0.0.1,localhost,internal.domain";

  # Select internationalisation properties.
  # i18n.defaultLocale = "en_US.UTF-8";
  # console = {
  #   font = "Lat2-Terminus16";
  #   keyMap = "us";
  #   useXkbConfig = true; # use xkb.options in tty.
  # };

  # Enable the X11 windowing system.
  # services.xserver.enable = true;
  #boot.loader.sytemd-boot.enable =true;
  boot = {
    supportedFilesystems = [ "zfs" ];  # [1][5]
    loader.grub.device = "/dev/sdb";
    initrd.verbose = true;
    plymouth.enable = false;
    consoleLogLevel = 7;
    initrd.supportedFilesystems = [ "zfs" ];
    zfs.forceImportRoot = true;       # [5][7]
    #loader = {
      #grub = lib.mkDefault {                         # Use GRUB for ZFS compatibility [1][4]
        #enable = false;
       loader.grub.zfsSupport = true;
       loader.grub.efiSupport = true;
       loader.grub.efiInstallAsRemovable = true;
      #};
  #    systemd-boot.enable = lib.mkForce true;
    #loader.efi.canTouchEfiVariables = true;
    #};
  };

  networking.hostId = "12345678";      # Generate via `head -c8 /etc/machine-id` [2][5]

  fileSystems."/" =    { device = "zroot/local/root"; fsType = "zfs"; };  # [3][6]
  fileSystems."/nix" = { device = "zroot/local/nix";  fsType = "zfs"; };  # [2][3]
  fileSystems."/home" ={ device = "zroot/safe/home";  fsType = "zfs"; };  # [1][4]
  fileSystems."/boot" ={ device = "/dev/disk/by-label/BOOT"; fsType = "vfat"; };  # [1][5]

  systemd.services.zfs-rollback = {
        description = "Rollback root to blank snapshot";
        after = [ "zfs-import.target" ];
        wantedBy = [ "multi-user.target" ];
        script = "zfs rollback -r rpool/local/root@blank";  # [3][6]
  };

  boot.initrd.network = {
  enable = true;
  postCommands = ''
    zpool import -a
    echo "zfs load-key -a; killall zfs" >> /root/.profile  # [1][6]
  '';
  };  # Unlock via: ssh -p 2222 root@host

  time.timeZone = "America/Los_Angeles";
  networking.hostName = "YellowStone";

  environment.systemPackages = with pkgs; [
    curl
    wget
    vim
    git
  ];

  services.openssh.enable = true;

  nix.settings.experimental-features = [ "nix-command" "flakes" ];
  nixpkgs.config.allowUnfree = true;

   i18n = {
    defaultLocale = "en_US.UTF-8";
    extraLocaleSettings = {
      LC_ADDRESS = "en_US.UTF-8";
      LC_IDENTIFICATION = "en_US.UTF-8";
      LC_MEASUREMENT = "en_US.UTF-8";
      LC_MONETARY = "en_US.UTF-8";
      LC_NAME = "en_US.UTF-8";
      LC_NUMERIC = "en_US.UTF-8";
      LC_PAPER = "en_US.UTF-8";
      LC_TELEPHONE = "en_US.UTF-8";
      LC_TIME = "en_US.UTF-8";
    };
  };

  users = {
    mutableUsers = false;
    users."nixos" = {
      isNormalUser = true;
      initialPassword = "hai";
      extraGroups = [ "wheel" ];
    };
  };

  # Enable passwordless sudo.
  security.sudo.extraRules= [
    {  users = [ "nixos" ];
      commands = [
         { command = "ALL" ;
           options= [ "NOPASSWD" ];
        }
      ];
    }
  ];


  # Configure keymap in X11
  # services.xserver.xkb.layout = "us";
  # services.xserver.xkb.options = "eurosign:e,caps:escape";

  # Enable CUPS to print documents.
  # services.printing.enable = true;

  # Enable sound.
  # hardware.pulseaudio.enable = true;
  # OR
  # services.pipewire = {
  #   enable = true;
  #   pulse.enable = true;
  # };

  # Enable touchpad support (enabled default in most desktopManager).
  # services.libinput.enable = true;

  # Define a user account. Don't forget to set a password with ‘passwd’.
  # users.users.alice = {
  #   isNormalUser = true;
  #   extraGroups = [ "wheel" ]; # Enable ‘sudo’ for the user.
  #   packages = with pkgs; [
  #     tree
  #   ];
  # };

  # programs.firefox.enable = true;

  # List packages installed in system profile. To search, run:
  # $ nix search wget
  # environment.systemPackages = with pkgs; [
  #   vim # Do not forget to add an editor to edit configuration.nix! The Nano editor is also installed by default.
  #   wget
  # ];

  # Some programs need SUID wrappers, can be configured further or are
  # started in user sessions.
  # programs.mtr.enable = true;
  # programs.gnupg.agent = {
  #   enable = true;
  #   enableSSHSupport = true;
  # };

  # List services that you want to enable:

  # Enable the OpenSSH daemon.
  # services.openssh.enable = true;

  # Open ports in the firewall.
  # networking.firewall.allowedTCPPorts = [ ... ];
  # networking.firewall.allowedUDPPorts = [ ... ];
  # Or disable the firewall altogether.
  # networking.firewall.enable = false;

  # Copy the NixOS configuration file and link it from the resulting system
  # (/run/current-system/configuration.nix). This is useful in case you
  # accidentally delete configuration.nix.
  # system.copySystemConfiguration = true;

  # This option defines the first version of NixOS you have installed on this particular machine,
  # and is used to maintain compatibility with application data (e.g. databases) created on older NixOS versions.
  #
  # Most users should NEVER change this value after the initial install, for any reason,
  # even if you've upgraded your system to a new NixOS release.
  #
  # This value does NOT affect the Nixpkgs version your packages and OS are pulled from,
  # so changing it will NOT upgrade your system - see https://nixos.org/manual/nixos/stable/#sec-upgrading for how
  # to actually do that.
  #
  # This value being lower than the current NixOS release does NOT mean your system is
  # out of date, out of support, or vulnerable.
  #
  # Do NOT change this value unless you have manually inspected all the changes it would make to your configuration,
  # and migrated your data accordingly.
  #
  # For more information, see `man configuration.nix` or https://nixos.org/manual/nixos/stable/options#opt-system.stateVersion .
  system.stateVersion = "24.11"; # Did you read the comment?

}
