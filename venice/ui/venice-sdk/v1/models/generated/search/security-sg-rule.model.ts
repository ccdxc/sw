/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { SecurityProtoPort, ISecurityProtoPort } from './security-proto-port.model';
import { SecuritySGRule_action,  } from './enums';

export interface ISecuritySGRule {
    'apps'?: Array<string>;
    'proto-ports'?: Array<ISecurityProtoPort>;
    'action': SecuritySGRule_action;
    'from-ip-addresses'?: Array<string>;
    'to-ip-addresses'?: Array<string>;
    'from-security-groups'?: Array<string>;
    'to-security-groups'?: Array<string>;
}


export class SecuritySGRule extends BaseModel implements ISecuritySGRule {
    'apps': Array<string> = null;
    'proto-ports': Array<SecurityProtoPort> = null;
    'action': SecuritySGRule_action = null;
    'from-ip-addresses': Array<string> = null;
    'to-ip-addresses': Array<string> = null;
    'from-security-groups': Array<string> = null;
    'to-security-groups': Array<string> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'apps': {
            type: 'Array<string>'
        },
        'proto-ports': {
            type: 'object'
        },
        'action': {
            enum: SecuritySGRule_action,
            default: 'PERMIT',
            type: 'string'
        },
        'from-ip-addresses': {
            type: 'Array<string>'
        },
        'to-ip-addresses': {
            type: 'Array<string>'
        },
        'from-security-groups': {
            type: 'Array<string>'
        },
        'to-security-groups': {
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecuritySGRule.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecuritySGRule.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecuritySGRule.propInfo[prop] != null &&
                        SecuritySGRule.propInfo[prop].default != null &&
                        SecuritySGRule.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['apps'] = new Array<string>();
        this['proto-ports'] = new Array<SecurityProtoPort>();
        this['from-ip-addresses'] = new Array<string>();
        this['to-ip-addresses'] = new Array<string>();
        this['from-security-groups'] = new Array<string>();
        this['to-security-groups'] = new Array<string>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['apps'] != null) {
            this['apps'] = values['apps'];
        } else if (fillDefaults && SecuritySGRule.hasDefaultValue('apps')) {
            this['apps'] = [ SecuritySGRule.propInfo['apps'].default];
        } else {
            this['apps'] = [];
        }
        if (values) {
            this.fillModelArray<SecurityProtoPort>(this, 'proto-ports', values['proto-ports'], SecurityProtoPort);
        } else {
            this['proto-ports'] = [];
        }
        if (values && values['action'] != null) {
            this['action'] = values['action'];
        } else if (fillDefaults && SecuritySGRule.hasDefaultValue('action')) {
            this['action'] = <SecuritySGRule_action>  SecuritySGRule.propInfo['action'].default;
        } else {
            this['action'] = null
        }
        if (values && values['from-ip-addresses'] != null) {
            this['from-ip-addresses'] = values['from-ip-addresses'];
        } else if (fillDefaults && SecuritySGRule.hasDefaultValue('from-ip-addresses')) {
            this['from-ip-addresses'] = [ SecuritySGRule.propInfo['from-ip-addresses'].default];
        } else {
            this['from-ip-addresses'] = [];
        }
        if (values && values['to-ip-addresses'] != null) {
            this['to-ip-addresses'] = values['to-ip-addresses'];
        } else if (fillDefaults && SecuritySGRule.hasDefaultValue('to-ip-addresses')) {
            this['to-ip-addresses'] = [ SecuritySGRule.propInfo['to-ip-addresses'].default];
        } else {
            this['to-ip-addresses'] = [];
        }
        if (values && values['from-security-groups'] != null) {
            this['from-security-groups'] = values['from-security-groups'];
        } else if (fillDefaults && SecuritySGRule.hasDefaultValue('from-security-groups')) {
            this['from-security-groups'] = [ SecuritySGRule.propInfo['from-security-groups'].default];
        } else {
            this['from-security-groups'] = [];
        }
        if (values && values['to-security-groups'] != null) {
            this['to-security-groups'] = values['to-security-groups'];
        } else if (fillDefaults && SecuritySGRule.hasDefaultValue('to-security-groups')) {
            this['to-security-groups'] = [ SecuritySGRule.propInfo['to-security-groups'].default];
        } else {
            this['to-security-groups'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'apps': new FormControl(this['apps']),
                'proto-ports': new FormArray([]),
                'action': new FormControl(this['action'], [required, enumValidator(SecuritySGRule_action), ]),
                'from-ip-addresses': new FormControl(this['from-ip-addresses']),
                'to-ip-addresses': new FormControl(this['to-ip-addresses']),
                'from-security-groups': new FormControl(this['from-security-groups']),
                'to-security-groups': new FormControl(this['to-security-groups']),
            });
            // generate FormArray control elements
            this.fillFormArray<SecurityProtoPort>('proto-ports', this['proto-ports'], SecurityProtoPort);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['apps'].setValue(this['apps']);
            this.fillModelArray<SecurityProtoPort>(this, 'proto-ports', this['proto-ports'], SecurityProtoPort);
            this._formGroup.controls['action'].setValue(this['action']);
            this._formGroup.controls['from-ip-addresses'].setValue(this['from-ip-addresses']);
            this._formGroup.controls['to-ip-addresses'].setValue(this['to-ip-addresses']);
            this._formGroup.controls['from-security-groups'].setValue(this['from-security-groups']);
            this._formGroup.controls['to-security-groups'].setValue(this['to-security-groups']);
        }
    }
}

