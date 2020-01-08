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
import { AuthService } from '@app/services/auth.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TestingUtility } from '@app/common/TestingUtility';
import { LocalComponent } from './local.component';


describe('LocalComponent', () => {
  let component: LocalComponent;
  let fixture: ComponentFixture<LocalComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [LocalComponent],
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
        AuthService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(LocalComponent);
    component = fixture.componentInstance;
  });

  it('should display toggle based on input', () => {
    component.localData = { enabled: true };
    fixture.detectChanges();
    let toggle = fixture.debugElement.queryAll(By.css('.mat-checked'));
    expect(toggle.length).toBe(0);

    component.localData = { enabled: false };
    fixture.detectChanges();
    toggle = fixture.debugElement.queryAll(By.css('.mat-checked'));
    expect(toggle.length).toBe(0);
  });

  describe('RBAC', () => {

    it('should display arrows and rank', () => {
      TestingUtility.addPermissions([UIRolePermissions.authauthenticationpolicy_update]);
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
    });
  });
});
