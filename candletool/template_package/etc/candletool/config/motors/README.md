To update the configs from the CANdle-SDK repository please use the following command:

Call once:

 ```
 git remote add motor_configs https://github.com/mabrobotics/motor_configs.git 
 ```

Than run:
```
git subtree pull \
  --prefix=candletool/template_package/etc/candletool/config/motors \
  motor_configs main \
  --squash
```