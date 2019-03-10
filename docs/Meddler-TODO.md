Meddler-TODO

`inherited deprecated SIG;` to mark parent method in interface deprecated, and issue a warning when called.

`inherited dropped SIG;` to mark parent method dropped and not available anymore. Will error if invoked through this version of interface. This method will not be inherited in the next version.

This makes interface evolution naturally self-documenting.

Type and exception definitions can also be deprecated or dropped (?).
