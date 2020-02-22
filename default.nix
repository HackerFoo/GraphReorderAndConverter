# { stdenv, fetchurl }:
with import <nixpkgs> {};

let
  stdenv_gcc9 = overrideCC stdenv gcc9;
  stdenv_gcc8 = overrideCC stdenv gcc8;
  stdenv_gcc7 = overrideCC stdenv gcc7;
  stdenv_gcc6 = overrideCC stdenv gcc6;
in
stdenv.mkDerivation rec {
  name = "GraphReorderAndConvertor-${version}";
  version = "git";
  src = fetchGit {
   url = "https://github.com/RapidsAtHKUST/GraphReorderAndConverter";
  };

  #doCheck = true;

  buildInputs = [ boost.dev cmake numactl metis tbb hwloc capnproto pkgconfig ];
}
