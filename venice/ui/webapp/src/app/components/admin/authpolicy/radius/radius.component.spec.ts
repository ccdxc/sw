import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';


import { SharedModule } from '@app/components/shared/shared.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';

import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { By } from '@angular/platform-browser';
import { MessageService } from '@app/services/message.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TestingUtility } from '@app/common/TestingUtility';
import { RadiusComponent } from './radius.component';
import { AuthRadius } from '@sdk/v1/models/generated/auth';

describe('RadiusComponent', () => {
  let component: RadiusComponent;
  let fixture: ComponentFixture<RadiusComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [RadiusComponent],
      imports: [
        MaterialdesignModule,
        RouterTestingModule,
        HttpClientTestingModule,
        SharedModule,
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        PrimengModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService,
        UIConfigsService,
        LicenseService,
        AuthService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(RadiusComponent);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    fixture.detectChanges();
    expect(component).toBeTruthy();
  });

  describe('RBAC', () => {
    beforeEach(() => {
      TestingUtility.removeAllPermissions();
    });

    it('should display arrows and rank', () => {
      // if radiusData is blank, it goes to create form which has no rank
      component.radiusData = new AuthRadius({ enabled: true });
      TestingUtility.addPermissions([UIRolePermissions.authauthenticationpolicy_update, UIRolePermissions.authauthenticationpolicy_delete]);
      fixture.detectChanges();
      const spy = spyOn(component.changeAuthRank, 'emit');
      // Current rank isn't set, shouldn't show anything
      let rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(0);

      // NO ARROWS
      component.currentRank = 0;
      component.numRanks = 1;
      fixture.detectChanges();
      let arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(1);
      expect(rank[0].nativeElement.innerText).toContain('1');


      // UP ARROW ONLY
      component.currentRank = 1;
      component.numRanks = 2;
      fixture.detectChanges();
      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(1);
      arrow_container[0].children[0].nativeElement.click();
      expect(spy).toHaveBeenCalledTimes(1);
      expect(spy).toHaveBeenCalledWith(0);

      rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(1);
      expect(rank[0].nativeElement.innerText).toContain('2');

      // DOWN ARROW ONLY
      spy.calls.reset();
      component.currentRank = 0;
      component.numRanks = 2;
      fixture.detectChanges();
      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(1);
      arrow_container[0].children[0].nativeElement.click();
      expect(spy).toHaveBeenCalledTimes(1);
      expect(spy).toHaveBeenCalledWith(1);

      rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(1);
      expect(rank[0].nativeElement.innerText).toContain('1');

      // BOTH ARROWS
      component.currentRank = 1;
      component.numRanks = 3;
      fixture.detectChanges();
      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(1);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(1);

      rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(1);
      expect(rank[0].nativeElement.innerText).toContain('2');

      // If we are in edit mode, arrows and rank are not visible
      // hover event
      fixture.debugElement.triggerEventHandler('mouseenter', null);
      fixture.detectChanges();
      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(3);
      arrow_container[0].children[0].nativeElement.click();
      fixture.detectChanges();

      // should be in edit mode
      const saveButton = fixture.debugElement.query(By.css('.authpolicy-save'));
      expect(saveButton).toBeTruthy();
      const cancelButton = fixture.debugElement.query(By.css('.authpolicy-cancel'));
      expect(cancelButton).toBeTruthy();

      // rank and arrows not visible
      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(0);

      rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(0);
    });

    it('should create new', () => {
      TestingUtility.addPermissions([UIRolePermissions.authauthenticationpolicy_update, UIRolePermissions.authauthenticationpolicy_delete]);
      fixture.detectChanges();
      // overlay exists
      const overlay = fixture.debugElement.query(By.css('.radius-overlay'));
      expect(overlay).toBeDefined();

      // create button exists
      const createButton = fixture.debugElement.query(By.css('.radius-create'));
      expect(createButton).toBeDefined();

      // arrow and ranks should't exist
      let arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      const rank = fixture.debugElement.query(By.css('.authpolicy-rank'));
      expect(rank).toBeNull();
    });

    it('should not create new', () => {
      TestingUtility.addPermissions([UIRolePermissions.authauthenticationpolicy_read]);
      fixture.detectChanges();

      // create button should not exist
      const createButton = fixture.debugElement.queryAll(By.css('.radius-create'));
      expect(createButton.length).toBe(0);

      // arrows, edit and delete button should not exist
      // .authpolicy-arrow-up length 0 implies no edit, delete and arrow button are present
      let arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(0);
    });
  });
});
