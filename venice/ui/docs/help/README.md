# Venice UI Help system

## Usage

#### File location
Markdown files can live anywhere under `ui/docs/help`. Each markdown file should have an id metadata:
```
---
id: authpolicy
---
```
To link to another markdown file, set the link as '%<markdown-id>'.

After adding or modifying any files run `make ui-autogen` from sw/.

### Image locations
Images must live under `ui/docs/help/images/`. Other markdown files can then by using `/images/path/to/image.png`

Ex. `![LDAP image](/images/ldap.png)`

## Adding in generated tables
The markdown file is be rendered as is except for `<load-table>`.
`<load-table>` allows you to add in tables of api fields/descriptions that are generated from the swagger files.

load-table options can be specified with key:value notation placed inside the tag.

Option | Description |
| ------ | ----------- |
| *group | Required. ApiGroup of object. |
| *obj   | Required. Object under apigroup to show. This doesn't need to be a top level object, see the swagger files to find the names of nested objects. |
| omit   | A comma separated list of fields to omit from the table |
| include | A list of fields to be rendered. All others will be omitted |
| omitHeader | Whether or not to omit the header of a table. Useful for combining two generated tables together. |

## Linking help docs with UI pages.

`urlMap.ts` contains a mapping of UI paths to help doc IDs. Update this map to see the doc show up in the UI.