/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { SecurityCertificateSpec_usages,  } from './enums';

export interface ISecurityCertificateSpec {
    'description'?: string;
    'usages': Array<SecurityCertificateSpec_usages>;
    'body'?: string;
    'trust-chain'?: string;
}


export class SecurityCertificateSpec extends BaseModel implements ISecurityCertificateSpec {
    'description': string = null;
    /** Usage can be "client", "server" or "trust-root" in any combination.
    A "server" certificate is used by a server to authenticate itself to the client
    A "client" certificate is used by a client to authenticate itself to a server
    A "trust-root" certificate is self-signed and is only used to validate
    certificates presented by peers.
    "client" and "server" certificates are always accompanied by a private key,
    whereas "trust-root"-only certificates are not. */
    'usages': Array<SecurityCertificateSpec_usages> = null;
    'body': string = null;
    /** Trust chain of the certificate in PEM encoding.
    These certificates are treated opaquely. We do not process them in any way
    other than decoding them for informational purposes. */
    'trust-chain': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'description': {
            type: 'string'
        },
        'usages': {
            enum: SecurityCertificateSpec_usages,
            default: 'Server',
            description:  'Usage can be &quot;client&quot;, &quot;server&quot; or &quot;trust-root&quot; in any combination. A &quot;server&quot; certificate is used by a server to authenticate itself to the client A &quot;client&quot; certificate is used by a client to authenticate itself to a server A &quot;trust-root&quot; certificate is self-signed and is only used to validate certificates presented by peers. &quot;client&quot; and &quot;server&quot; certificates are always accompanied by a private key, whereas &quot;trust-root&quot;-only certificates are not.',
            type: 'Array<string>'
        },
        'body': {
            type: 'string'
        },
        'trust-chain': {
            description:  'Trust chain of the certificate in PEM encoding. These certificates are treated opaquely. We do not process them in any way other than decoding them for informational purposes.',
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecurityCertificateSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecurityCertificateSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecurityCertificateSpec.propInfo[prop] != null &&
                        SecurityCertificateSpec.propInfo[prop].default != null &&
                        SecurityCertificateSpec.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['usages'] = new Array<SecurityCertificateSpec_usages>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['description'] != null) {
            this['description'] = values['description'];
        } else if (fillDefaults && SecurityCertificateSpec.hasDefaultValue('description')) {
            this['description'] = SecurityCertificateSpec.propInfo['description'].default;
        } else {
            this['description'] = null
        }
        if (values && values['usages'] != null) {
            this['usages'] = values['usages'];
        } else if (fillDefaults && SecurityCertificateSpec.hasDefaultValue('usages')) {
            this['usages'] = [ SecurityCertificateSpec.propInfo['usages'].default];
        } else {
            this['usages'] = [];
        }
        if (values && values['body'] != null) {
            this['body'] = values['body'];
        } else if (fillDefaults && SecurityCertificateSpec.hasDefaultValue('body')) {
            this['body'] = SecurityCertificateSpec.propInfo['body'].default;
        } else {
            this['body'] = null
        }
        if (values && values['trust-chain'] != null) {
            this['trust-chain'] = values['trust-chain'];
        } else if (fillDefaults && SecurityCertificateSpec.hasDefaultValue('trust-chain')) {
            this['trust-chain'] = SecurityCertificateSpec.propInfo['trust-chain'].default;
        } else {
            this['trust-chain'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'description': CustomFormControl(new FormControl(this['description']), SecurityCertificateSpec.propInfo['description'].description),
                'usages': CustomFormControl(new FormControl(this['usages']), SecurityCertificateSpec.propInfo['usages'].description),
                'body': CustomFormControl(new FormControl(this['body']), SecurityCertificateSpec.propInfo['body'].description),
                'trust-chain': CustomFormControl(new FormControl(this['trust-chain']), SecurityCertificateSpec.propInfo['trust-chain'].description),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['description'].setValue(this['description']);
            this._formGroup.controls['usages'].setValue(this['usages']);
            this._formGroup.controls['body'].setValue(this['body']);
            this._formGroup.controls['trust-chain'].setValue(this['trust-chain']);
        }
    }
}

