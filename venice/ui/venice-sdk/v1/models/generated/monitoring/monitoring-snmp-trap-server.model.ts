/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringSNMPTrapServer_version,  } from './enums';
import { MonitoringAuthConfig, IMonitoringAuthConfig } from './monitoring-auth-config.model';
import { MonitoringPrivacyConfig, IMonitoringPrivacyConfig } from './monitoring-privacy-config.model';

export interface IMonitoringSNMPTrapServer {
    'host'?: string;
    'port'?: string;
    'version'?: MonitoringSNMPTrapServer_version;
    'community-or-user'?: string;
    'auth-config'?: IMonitoringAuthConfig;
    'privacy-config'?: IMonitoringPrivacyConfig;
}


export class MonitoringSNMPTrapServer extends BaseModel implements IMonitoringSNMPTrapServer {
    /** Host where the trap needs to be sent. */
    'host': string = null;
    /** Port on the Host where the trap needs to be sent, default is 162. */
    'port': string = null;
    'version': MonitoringSNMPTrapServer_version = null;
    /** CommunityOrUser contains community string for v2c, user for v3. */
    'community-or-user': string = null;
    /** AuthConfig contains the configuration for authentication, valid only for v3. */
    'auth-config': MonitoringAuthConfig = null;
    /** PrivacyConfig contains the configuration for encryption, valid only for v3. */
    'privacy-config': MonitoringPrivacyConfig = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'host': {
            description:  'Host where the trap needs to be sent.',
            type: 'string'
        },
        'port': {
            default: '162',
            description:  'Port on the Host where the trap needs to be sent, default is 162.',
            type: 'string'
        },
        'version': {
            enum: MonitoringSNMPTrapServer_version,
            default: 'V2C',
            type: 'string'
        },
        'community-or-user': {
            description:  'CommunityOrUser contains community string for v2c, user for v3.',
            type: 'string'
        },
        'auth-config': {
            description:  'AuthConfig contains the configuration for authentication, valid only for v3.',
            type: 'object'
        },
        'privacy-config': {
            description:  'PrivacyConfig contains the configuration for encryption, valid only for v3.',
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringSNMPTrapServer.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringSNMPTrapServer.propInfo[prop] != null &&
                        MonitoringSNMPTrapServer.propInfo[prop].default != null &&
                        MonitoringSNMPTrapServer.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['auth-config'] = new MonitoringAuthConfig();
        this['privacy-config'] = new MonitoringPrivacyConfig();
        this.setValues(values);
    }

    /**
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values && values['host'] != null) {
            this['host'] = values['host'];
        } else if (MonitoringSNMPTrapServer.hasDefaultValue('host')) {
            this['host'] = MonitoringSNMPTrapServer.propInfo['host'].default;
        }
        if (values && values['port'] != null) {
            this['port'] = values['port'];
        } else if (MonitoringSNMPTrapServer.hasDefaultValue('port')) {
            this['port'] = MonitoringSNMPTrapServer.propInfo['port'].default;
        }
        if (values && values['version'] != null) {
            this['version'] = values['version'];
        } else if (MonitoringSNMPTrapServer.hasDefaultValue('version')) {
            this['version'] = <MonitoringSNMPTrapServer_version>  MonitoringSNMPTrapServer.propInfo['version'].default;
        }
        if (values && values['community-or-user'] != null) {
            this['community-or-user'] = values['community-or-user'];
        } else if (MonitoringSNMPTrapServer.hasDefaultValue('community-or-user')) {
            this['community-or-user'] = MonitoringSNMPTrapServer.propInfo['community-or-user'].default;
        }
        if (values) {
            this['auth-config'].setValues(values['auth-config']);
        }
        if (values) {
            this['privacy-config'].setValues(values['privacy-config']);
        }
    }




    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'host': new FormControl(this['host']),
                'port': new FormControl(this['port']),
                'version': new FormControl(this['version'], [enumValidator(MonitoringSNMPTrapServer_version), ]),
                'community-or-user': new FormControl(this['community-or-user']),
                'auth-config': this['auth-config'].$formGroup,
                'privacy-config': this['privacy-config'].$formGroup,
            });
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['host'].setValue(this['host']);
            this._formGroup.controls['port'].setValue(this['port']);
            this._formGroup.controls['version'].setValue(this['version']);
            this._formGroup.controls['community-or-user'].setValue(this['community-or-user']);
            this['auth-config'].setFormGroupValues();
            this['privacy-config'].setFormGroupValues();
        }
    }
}

