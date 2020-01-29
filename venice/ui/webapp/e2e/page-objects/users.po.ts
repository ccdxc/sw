import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { Alert } from 'selenium-webdriver';
import { E2EuiTools } from './E2EuiTools';

import { IAuthRoleBinding, IAuthUser, IAuthRole , AuthRoleBinding, AuthUser, AuthRole  } from '@sdk/v1/models/generated/auth';

export class UsersPage {
    toolbarCSS: string = '.toolbarcomponent-toolbar ';
    addRecordButton = '.global-button-primary.users-toolbar-button.users-add-button';
    refreshButton = '.toolbar-button.global-button-primary.users-toolbar-button.users-toolbar-button-refresh';

    constructor() {
    }

    navigateTo() {
        return browser.get('/#/settings/users');
    }

    async verifyPage() {
        const EC = protractor.ExpectedConditions;
        const usersPageComponent = element(by.css('app-users'));
        const urlEC = EC.urlContains('/settings/users');
        const componentEC = EC.presenceOf(usersPageComponent);
        await browser.wait(EC.and(urlEC, componentEC));
    }

    async _openToolbarRBACMgmtDropDownPanel() {
        // browser.wait(element(by.css('ui-toast-summary')).
        const toolbarDropDowpArrow = await element(by.css(this.toolbarCSS + ' ' + '.ui-dropdown-trigger.ui-corner-right .ui-dropdown-trigger-icon.ui-clickable'));
        await toolbarDropDowpArrow.click();
    }

    async _setToolbarRBACMgmtDropDownValue(key: string,  myKeyCSS: string = null) {
        const dropDownPanelCSS = '.ng-trigger.ng-trigger-overlayAnimation.ui-dropdown-panel.ui-widget .ui-dropdown-items-wrapper';
        const keyCSS = (myKeyCSS) ? myKeyCSS : this.toolbarCSS + dropDownPanelCSS + ' .ui-dropdown-item';
        await this._openToolbarRBACMgmtDropDownPanel();
        browser.wait(element(by.css(dropDownPanelCSS)).isDisplayed());  // important it waits for menu dropdown panel appear on screen
        const keysElements = await element.all(by.css(keyCSS));
        const keyTexts = await element.all(by.css(keyCSS)).getText();
        const actionOption = E2EuiTools.findElementFinder(keysElements, keyTexts, key);
        await actionOption.click();
      }

    async _verifyRecords(type: string, cssLine: string) {
      const records = await element.all(by.css('.usertemplate-div-card-username'));
      expect( records.length > 0).toBeTruthy();
    }

    async verifyUsers() {
        await this._setToolbarRBACMgmtDropDownValue('user');
        await this._verifyRecords('user', '.usertemplate-div-card-username');
    }

    async verifyRoles() {
        await this._setToolbarRBACMgmtDropDownValue('role');
        await this._verifyRecords('role', '.usertemplate-div-card-username');
    }

    async verifyRolebindings() {
        await this._setToolbarRBACMgmtDropDownValue('rolebinding');
        await this._verifyRecords('role', '.usertemplate-div-card-username');
    }

    async _verifyOneAuthObject(type: string, authObject: any, isToverifyExist: boolean) {
        await this._setToolbarRBACMgmtDropDownValue(type);
        await browser.sleep(1500);
        const authObjectBlockCSS = this.getAuthObjectBlockCSS(type, authObject.meta.name);
        const record: ElementFinder = await element(by.css(authObjectBlockCSS));
        if (isToverifyExist) {
            expect(record.isPresent()).toBeTruthy();
        } else {
            expect(record.isPresent()).not.toBeTruthy();
        }
    }

    async verifyUserExist(authUser: AuthUser, isExist: boolean = true) {
        await this._verifyOneAuthObject('user', authUser, isExist);
    }

    async verifyRoleExist(authRole: AuthRole, isExist: boolean = true) {
        await this._verifyOneAuthObject('role', authRole, isExist);
    }

    async _pressAddButton() {
        await E2EuiTools.clickElement(this.addRecordButton);
    }

    async refreshUIPage() {
        await E2EuiTools.clickElement(this.refreshButton);
    }

    async addRole(authRole: AuthRole) {
        await this._setToolbarRBACMgmtDropDownValue('role');
        await browser.sleep(3000); // wait for data load-up.
        await this._pressAddButton();
        await browser.sleep(1500);
        // set role name
        await element(by.css('.newrole-input-item.newrole-fullname.ui-inputtext')).sendKeys(authRole.meta.name);
        // set role permission
        for (let i = 0; i < authRole.spec.permissions.length; i ++ ) {
                const permission = authRole.spec.permissions[i];
                const tenantCSS = '.newrole-permission .newrole-host.ui-inputtext';
                await E2EuiTools.setInputBoxValue(tenantCSS, permission['resource-tenant']);
                const groupDropdownArrowCSS = '.newrole-permission p-dropdown[formcontrolname="resource-group"] .ui-dropdown-trigger.ui-corner-right .ui-dropdown-trigger-icon.ui-clickable';
                await E2EuiTools.setDropdownValue(permission['resource-group'], i, groupDropdownArrowCSS );
                const kindDropdownArrowCSS = '.newrole-permission p-dropdown[formcontrolname="resource-kind"] .ui-dropdown-trigger.ui-corner-right .ui-dropdown-trigger-icon.ui-clickable';
                await browser.sleep(1000);  // wait for group change code finish.
                await E2EuiTools.setDropdownValue(permission['resource-kind'], i, kindDropdownArrowCSS );
                await browser.sleep(1000);
                // set permit action
                const actionListCSS = '.newrole-permission:nth-child('  + (i + 1 ) + ')  p-listbox .ui-listbox-list-wrapper .ui-listbox-item > span';
                await browser.sleep(1000);
                await E2EuiTools.setListBoxValue(permission.actions, actionListCSS );
        }
        const saveRoleButtonCSS = '.global-button-primary.newrole-addnewuser.newrole-addnewuser-save';
        await E2EuiTools.clickElement(saveRoleButtonCSS);
    }

    async addUser(authUser: AuthUser, newuserRoleBindings: string[]) {
        await this._setToolbarRBACMgmtDropDownValue('user');
        await browser.sleep(3000); // wait for data load-up.
        await this._pressAddButton();
        await browser.sleep(1500);
        // set  user login name
        await element(by.css('.newuser-input-item.newuser-loginname.ui-inputtext')).sendKeys(authUser.meta.name);
        // set  user full name
        await element(by.css('.newuser-input-item.newuser-fullname.ui-inputtext')).sendKeys(authUser.spec.fullname);
        // set user email
        await element(by.css('.newuser-input-item.newuser-email.ui-inputtext')).sendKeys(authUser.spec.email);
        // set user password
        await element(by.css('.newuser-input-item.newuser-Password.ui-inputtext')).sendKeys(authUser.spec.password);
        const newUserRolebindingDDArrowCSS = '.newuser .newuser-rolebinding .ui-multiselect-trigger-icon.ui-clickable';
        await E2EuiTools.setMultiSelectDropdownValue(newuserRoleBindings, 0, newUserRolebindingDDArrowCSS);
        const saveUserButtonCSS = '.global-button-primary.newuser-addnewuser.newuser-addnewuser-save';
        await E2EuiTools.clickElement(saveUserButtonCSS);
    }

    async addRecords(authRole: AuthRole, authUser: AuthUser, newuserRoleBindings: string[] = null) {
       await this.addRole(authRole);
       await browser.sleep(5000); // wait for save-role settle down,toaster dismisses.
       await this.verifyRoleExist(authRole, true);
       if (!newuserRoleBindings) {
         newuserRoleBindings = [authRole.meta.name + '_binding'];
       }
       await this.refreshUIPage();
       await browser.sleep(1000);
       await this.addUser(authUser, newuserRoleBindings);
       await browser.sleep(5000); // wait for save-role settle down,toaster dismisses.
       await this.verifyUserExist(authUser, true);
    }


    /**
     * Every auth-object (user, role, rolebinding block) has class like 'usertemplate-div_user_user-2990'
     * where 'user-2990' is auth-object.meta.name
     * @param authObject
     * @param type
     */
    async _deleteRecordHelper(authObject: AuthRole|AuthUser|AuthRoleBinding, type: string) {
        const authObjectBlockCSS = await this._setFocusToAuthObjectUIElement(type, authObject); // put focus to the auth-object block
        const deletebtnCSS = authObjectBlockCSS + ' ' + '.usertemplate-div-card-icon-delete-button';
        await E2EuiTools.clickElement(deletebtnCSS);
        await E2EuiTools.clickConfirmAlertFirstButton();
    }

    private async _setFocusToAuthObjectUIElement(type: string, authObject: AuthRole | AuthUser | AuthRoleBinding) {
        await this._setToolbarRBACMgmtDropDownValue(type);
        await browser.sleep(1500);
        const authObjectBlockCSS = this.getAuthObjectBlockCSS(type, authObject.meta.name);
        await E2EuiTools.clickElement(authObjectBlockCSS); // put focus to the auth-object block
        await browser.sleep(1500);
        return authObjectBlockCSS;
    }

    private getAuthObjectBlockCSS(type: string, authObjectMetaName: string) {
        return '.usertemplate-div_' + type + '_' + authObjectMetaName;
    }

    async deleteRole(authRole: AuthRole) {
        await this._deleteRecordHelper(authRole, 'role');
    }

    async deleteUser(authUser: AuthUser) {
        await this._deleteRecordHelper(authUser, 'user');
    }


    async changeUserPassword(authUser: AuthUser, oldPassword: string, newPassword: string) {
        const authObjectBlockCSS = await this._setFocusToAuthObjectUIElement('user', authUser);
        const changePasswordBtnCSS = authObjectBlockCSS + ' ' + '.usertemplate-div-card-icon-changepassword-button';
        await E2EuiTools.clickElement(changePasswordBtnCSS);
        await browser.sleep(1000); // wait for the change password panel show up
        const oldPswordCSS = '.user-edit-input-item.user-edit-Password[formcontrolname="old-password"]';
        const newPswordCSS = '.user-edit-input-item.user-edit-Password[formcontrolname="new-password"]';
        await E2EuiTools.setInputBoxValue(oldPswordCSS, oldPassword);
        await E2EuiTools.setInputBoxValue(newPswordCSS, newPassword);
        await browser.sleep(1000);
        const saveButtonCSS = '.usertemplate-div-card-icon-operation-button.usertemplate-div-card-icon-save-button';
        await E2EuiTools.clickElement(saveButtonCSS);

    }



    async deleteRecords(authRole: AuthRole, authUser: AuthUser, newuserRoleBindings: string[] = null) {
        await this.deleteRole(authRole);
        await browser.sleep(5000); // wait for CRUD operation settle down,toaster dismisses.
        await this.verifyRoleExist(authRole, false);
        await this.refreshUIPage();
        await this.deleteUser(authUser);
        await browser.sleep(5000); // wait for CRUD operation settle down,toaster dismisses.
        await this.verifyUserExist(authUser, false);
     }
}
