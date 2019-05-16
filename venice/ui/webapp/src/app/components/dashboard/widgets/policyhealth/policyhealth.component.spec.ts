import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { PrimengModule } from '@app/lib/primeng.module';

import { PolicyhealthComponent } from './policyhealth.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SharedModule } from '@app/components/shared/shared.module';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';

describe('PolicyhealthComponent', () => {
  let component: PolicyhealthComponent;
  let fixture: ComponentFixture<PolicyhealthComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [PolicyhealthComponent],
      imports: [
        PrimengModule,
        RouterTestingModule,
        HttpClientTestingModule,
        MaterialdesignModule,
        NoopAnimationsModule,
        SharedModule
      ],
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(PolicyhealthComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
