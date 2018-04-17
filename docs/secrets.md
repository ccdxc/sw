# Secrets Management

## Introduction
A "secret" is a piece of user-supplied data that has special confidentiality
requirements. Examples of secrets are credentials to access 3rd party resources
like passwords, private keys, ssh keys and API keys.
User provides secrets in plaintext for the system to use, but expects that
system does not expose them to outside observers. For example, secrets must not
be shown in UIs and should not appear in legible forms in system logs or
configuration backups.

This document presents the design of a generic framework for handling secrets in
Venice.  It addresses the following topics:

1. how secrets are identified in object model definitions
2. how controllers and agents access secrets
3. how secrets are protected at rest
4. how encryption keys are managed
5. how the framework can be extended to support external Key Management Systems
   (KMS)

In the last section we also briefly describe how Kubernetes encrypts secrets at
rest and the key differences with our design.

Note that the system handles various data types that match the definition of
"secret" presented above but do not make use of this framework. These includes:

* private keys for user-provided workload certificates, as they take advantage
  of NAPLES secure key storage capabilities
* Venice user passwords, as they are managed with ad-hoc techniques

## Requirements and Goal
The section provides an overview of the requirements that the framework aims
to satisfy and the high-level goals that have driven the design.

### Confidentiality
Confidentiality is the most important requirement, as secrets may grant access
to critical systems (orchestrators, monitoring systems, etc.). The framework
uses only high-quality implementations of widely deployed cryptographic
algorithms with strong keys (256 bits for symmetric keys). Keys are generated
using NAPLES CSRNG (cryptographically-secure random number generator) if
available. Keys are rotated periodically.

### Integrity
Techniques like MAC (Message Authentication Code) are used to protect against
unauthorized modifications of the data. The need for integrity in the framework
is not entirely clear, given that system configuration is not integrity-checked
and an attacker with write access to the storage layer would be able to subvert
the entire system. However, being able to guarantee the integrity of a secret
stored offline (for example in a config backup) is desirable. Moreover, using
encryption without authentication is generally considered a bad practice and is
highly discouraged. For these reasons the framework employs algorithms that
provide both confidentiality and integrity.

### Availability
Secret data must enjoy the same high-availability and fault-tolerance properties
offered to non-secret configuration data. Keys must be readily available at all
times.

### Performance and Scale
We anticipate a small number of secrets (few hundreds at most) that are accessed
infrequently, so performance is not a primary concern. However, the framework is
generic and in the future may need to handle high-volume, frequently changing
data. The design does not impose any pre-defined limit on the number of stored
secrets and tries to keep latency and processing overhead negligible during
normal cluster operations.

### Ease of use for users
Secrets encryption is not meant to be a user-visible feature but simply an
implementation of best practices for the design of secure systems. Functionality
is always on and cannot be disabled. User does not have to provide any key,
password or other input either at system provisioning time, after reboot or
during normal operations. User does not have to supply any configuration
parameter (encryption algorithms, key sizes, rotation periods, etc.). By default
all the key-management duties (key generation, rotation, etc.) are automatically
performed by the system.
User has to provide configuration parameters only in the case in which
integration with external KMS systems is desired.

### Ease of use for developers
Developers' main responsibility is to indicate what fields contain secrets in
the object model definitions. The framework should transparently takes care of
all extra steps that are required to read and write secrets. The ApiServer
automatically zeroizes secret object fields before transmitting query results
and watch notifications to Api Gateway.

Developers are still responsible to make sure that secrets are not exposed via
non-secret object fields, logs, environment variables, etc. Memory holding
plaintext secrets should be zeroized and released as soon as the secret is
no longer needed.

### KMS Integration
The framework must be designed to integrate with external key management
systems. The actual integration work is not planned for phase 1.

## Design

### Object Model Extensions
Secret fields must be of type "string". A new field-level flag specifies that
the field contains a secret. In the future, secrets metadata may also contain
extra information such as what nodes or services are allowed to access it.
Secrets of type "bytes" are handled by converting them to "string" as needed.

### ApiServer
Before responding to a query on the Api Gateway interface, ApiServer checks the
metadata associated with each object (like proto message field types and flags)
and zeroizes the value of secret fields.
The same happens for watch notifications on the same interface.
Zeroization does not happen on the gRPC interface towards controllers.

ApiServer can implement access control on a per-service basis, because all
connections are mutually authenticated and it knows which service is at the
other end. We can start with a simple mechanism that allows access to all
secrets to Venice controllers and denies it to all other processes (NIC
agents, 3rd party controllers, etc.). Later we can improve it to control
access to individual secrets based on object model metadata.

Encryption of secrets is performed by the ApiServer before passing the object
to the KVStore layer. Encryption consists of replacing the secret plaintext
with the corresponding ciphertext as well as embedding in the field value
itself all the metadata required to perform decryption (for example, the key
name).
Conversely, decryption happens right after the object has been read from the
KVStore layer, before any further processing takes place, and consists in
stripping the metadata and replacing the ciphertext with the plaintext.

Encryption and decryption fit very well into the generic concept of
[transformer](https://github.com/kubernetes/kubernetes/blob/master/staging/src/k8s.io/apiserver/pkg/storage/value/transformer.go)
introduced in Kubernetes ApiServer. Transformers are Golang objects that
implement an interface with two callbacks: TransformToStorage and
TransformFromStorage. The callbacks are invoked before writing to etcd and right
after reading from etc. Transformers can be stateful or stateless and can be
chained. The actual callbacks implement the logic to decide when to transform
and how. Both inputs and outputs are strings.

The framework makes use of existing PreCommit and PostCommit hooks in ApiServer
and registers transformers on a per-kind basis. Transformers use object metadata
to find out which field values need to be processed.

### Key Hierarchy
Secrets are protected with a technique named _envelope encryption_, which uses
a 2-level key hierarchy. Data-Encryption Keys (DEKs) are used to encrypt the
data, while Key-Encryption Keys (KEKs) are used to encrypt DEKs. DEKs are stored
in encrypted form alongside the data they encrypt, whereas KEKs are stored
outside of the data store.

Encryption consists of the following steps:

1. Encrypt the data using a Data Encryption Key (DEK)
2. Encrypt the DEK using a Key-Encrypting-Key (KEK)
3. Store the encrypted DEK together with the data

Conversely, decryption consists of the following steps:

1. Decrypt the DEK using the KEK
2. Decrypt data using the decrypted DEK

DEKs can be cached for some period of time if the corresponding secrets are
accessed frequently.

Envelope encryption has the following advantages:

* Only the KEKs need to be protected and stored away from the data.
  If the KEKs reside on an external device, only DEKs (not data) needs to
  be sent over for encryption/decryption.
* Compromise of a DEK only exposes the data encrypted with that key.
* When a KEK is rotated, only DEKs (not the data) needs to be re-encrypted.

As of now we don't see the need to have a different KEK per-tenant, as these
secrets are consumed by infrastructure, not workloads. This choice simplifies
the design because it allows the system to generate, distribute and rotate
a single key whose lifecycle is not tied to the lifecyle of any tenant.

Venice controllers store the KEK in NAPLES, if it is available, or on disk.
After Phase 1 we will consider storing KEKs in external devices like HSM or KMS.

### Choice of the Encryption Algorithm
Symmetric encryption algorithm have different features and requirements and
there is no clearly superior choice. AES is by far the most popular one, as it
has been selected for many compliance frameworks adopted by government and
industry. It has also been around for a long time and has been heavily studied
by the academic community.

AES is a block cipher, so it can be used in different _modes_ that impart on it
significantly different characteristics. This
[post](https://blog.cryptographyengineering.com/2012/05/19/how-to-choose-authenticated-encryption/)
provides a good comparison of some of the most popular AES modes.
Below are summarized the main differences that are relevant for our purposes.

#### Features
Some modes only provide confidentiality, others provide confidentiality and
integrity, others provide confidentiality and integrity of the plaintext as well
as of "additional data" that is not part of the plaintext (useful for example
for protocol headers, version numbers, etc.)

#### Initialization Vectors (IV)
Most modes requires a user-provided IV. The IV does not need to be secret but it
must be a nonce (i.e. it has to be used exactly once with a given key). Some
modes fail catastrophically if the nonce is re-used, effectively leaking
plaintext as well as key material, whereas others only leak plaintext of the
instances where the nonce has been reused.
Some modes only require that the IV is a nonce, whereas others require that it
is unpredictable at encryption time (i.e. it must also be random). If the nonce
is predictable, the attacker can craft a special plaintext that will leak
information about the entire plaintext encrypted with the same key.

#### Performance
There are significant performance differences among modes. Moreover, some are
"on-line" and can start processing plaintext before it is fully available or
its length is known, whereas other cannot. As mentioned above, performance is
not a critical consideration for this scenario.

After considering all the pros and cons, we have selected AES-GCM with 256 bits
key and a random IV. The main reasons behind the choice are:

* it provides both confidentiality and integrity
* it is very popular and accepted in all compliance frameworks
* it is part of the Go standard crypto library

The main issue with AES-GCM is that it is not resistant to nonce reuse. In a
Venice cluster each quorum node can write to etcd and so needs to generate IVs.
Choosing IVs randomly simplifies the design considerably, as it means that
different quorum nodes do not need to synchronize state, but also implies that
after a while the probability of generating the same IV twice becomes
non-negligible. The AES IV is 96 bits, so NIST recommends no more than 2^32
writes using the same key and random nonces.
In our case 2^32 is a really large number, since we do not expect secrets to be
updated frequently. Still, if we want to be fully protected, we can perform DEK
rotation in the background or we can even generate a new key for every write,
removing the need for rotation altogether.

### Key Rotation
Rotation of DEKs is not explicitly supported but can be achieved by deleting
and re-creating the object containing secrets or rewriting a secret field.
Rotation of the Master Key happens periodically in the background. While
rotation is in progress, each quorum node has both the old and new key. New
secrets are encrypted using the new key, old secrets are decrypted by trying
both the old and the new key. When recovering from a split-brain situation,
the isolated node checks if rotation has happened while it was disconnected
and requests the new key if needed.

## Differences with Kubernetes Secrets Encryption
Kubernetes has implemented basic support for secrets encryption.
The requirements are similar for the most part, but there are also important
differences. This is a summary of the main differences and how they have
affected our design:

### Definition of Secret
In the Kubernetes case, secrets are consumed by workloads, whereas in Venice they
are consumed by infrastructure. For this reason, in Kubernetes they are
independent kinds with opaque data, whereas in Venice they are auxiliary data
for existing objects serving a well-known purpose.

### Scale
Kubernetes design is targeting large systems with up to 100K secrets. Every
time a new pod is scheduled on a node, it might be necessary to read the
secret and dispatch it to the node. In Venice, on the contrary, we are targeting
small number of secrets that do not get dispatched to NAPLES. Hence, performance
is an important consideration in Kubernetes design, whereas it is a secondary
one in our design.

### Configuration
Kubernetes design does not focus much on automation and assumes that users
will be comfortable manually generating, distributing and rotating keys as well
as re-starting ApiServer as needed. Moreover, it does not take a stance on the
choice of encryption parameters and passes it over to users.
Venice designs tries to hide configuration and operational details from user as
much as possible and aims for a "turn-key" solution that does not require any
expertise in cryptography.

### Kubernetes Secrets References
#### Design docs

* [Final Design Doc](https://github.com/kubernetes/community/pull/607/files)
* [A Roadmap For Secrets](https://docs.google.com/document/d/1T2y-9geg9EfHHtCDYTXptCa-F4kQ0RyiH-c_M1SyD0s/edit)
* [A Plan for Kubernetes Secrets](https://docs.google.com/document/d/1JAwPuZg47UhfRVlof-lMw08OJztunW8pvTNxDK3rCF8/edit)
* [Encrypt Secrets in Etcd](https://docs.google.com/document/d/1lFhPLlvkCo3XFC2xFDPSn0jAGpqKcCCZaNsBAv8zFdE/edit)
* [Vault based KMS provider for secrets in etcd3](https://docs.google.com/document/d/15-baW4i7qws1yxxIYjHXqKpk259ebauQbECQCpPD308/edit)
* [KMS Plugin API](https://docs.google.com/document/d/1S_Wgn-psI0Z7SYGvp-83ePte5oUNMr4244uanGLYUmw/edit)
* [Initial proposal](https://github.com/kubernetes/community/pull/454/files)

#### User documentation

* [Encrypting Secret Data at Rest](https://kubernetes.io/docs/tasks/administer-cluster/encrypt-data/)

#### Relevant PRs
* [Main PR](https://github.com/kubernetes/kubernetes/pull/41939)
* [Follow up PR](https://github.com/kubernetes/kubernetes/pull/46916)
* [Config options PR](https://github.com/kubernetes/kubernetes/pull/46460)
* [Envelope Transformers for KMS Integration PR](https://github.com/kubernetes/kubernetes/pull/49350)

#### Tracking Issues

* [Current](https://github.com/kubernetes/features/issues/92)
* [Initial](https://github.com/kubernetes/kubernetes/issues/12742)

## Implementation Notes

The Venice implementation consists of a generic "storage transformer" framework
and a specific transformer for secrets.

The storage transformers framework is described in [API](apidef.md) documentation.

The following files and directories contain the implementation of the Secret 
storage transformers and other additions to the existing API infrastructure:

* [api/integration/...](../api/integration)  
   Integration tests to verify secret encryption and zeroization logic for CRUD
   and Watch operations performed using gRPC and REST interfaces.

* [venice/apiserver/...](../venice/apiserver)  
   Storage transformers interface, runtime logic, tests, mocks

* [venice/utils/transformers/storage](../venice/utils/transformers/storage)  
   Field transformers interface, secrets transformer implementation, tests

* [venice/utils/apigen/plugins/...](../venice/utils/apigen/plugins)  
   Protoc extension definitions, parsing logic, plugin and templates additions, tests

