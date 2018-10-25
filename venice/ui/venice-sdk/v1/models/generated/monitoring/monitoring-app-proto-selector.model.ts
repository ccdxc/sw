/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IMonitoringAppProtoSelector {
    'ports'?: Array<string>;
    'applications'?: Array<string>;
}


export class MonitoringAppProtoSelector extends BaseModel implements IMonitoringAppProtoSelector {
    /** should be a valid layer3 or layer 4 protocol and port/type
     */
    'ports': Array<string> = null;
    'applications': Array<string> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'ports': {
            description:  'should be a valid layer3 or layer 4 protocol and port/type ',
            hint:  'tcp/1234, arp',
            type: 'Array<string>'
        },
        'applications': {
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringAppProtoSelector.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringAppProtoSelector.propInfo[prop] != null &&
                        MonitoringAppProtoSelector.propInfo[prop].default != null &&
                        MonitoringAppProtoSelector.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['ports'] = new Array<string>();
        this['applications'] = new Array<string>();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['ports'] != null) {
            this['ports'] = values['ports'];
        }
        if (values && values['applications'] != null) {
            this['applications'] = values['applications'];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'ports': new FormControl(this['ports']),
                'applications': new FormControl(this['applications']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['ports'].setValue(this['ports']);
            this._formGroup.controls['applications'].setValue(this['applications']);
        }
    }
}

