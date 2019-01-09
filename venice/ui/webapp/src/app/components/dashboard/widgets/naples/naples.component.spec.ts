import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { PrimengModule } from '@app/lib/primeng.module';

import { NaplesComponent } from './naples.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SharedModule } from '@app/components/shared/shared.module';

describe('NaplesComponent', () => {
  let component: NaplesComponent;
  let fixture: ComponentFixture<NaplesComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [NaplesComponent],
      imports: [
        PrimengModule,
        RouterTestingModule,
        HttpClientTestingModule,
        MaterialdesignModule,
        NoopAnimationsModule,
        SharedModule
      ],
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaplesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
