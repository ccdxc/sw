import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import {RouterTestingModule} from '@angular/router/testing';
import {HttpClientTestingModule} from '@angular/common/http/testing';
import {NoopAnimationsModule} from '@angular/platform-browser/animations';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {PrimengModule} from '@lib/primeng.module';
import {FormsModule} from '@angular/forms';
import { DscprofilesetterComponent } from './dscprofilesetter.component';

describe('DscprofilesetterComponent', () => {
  let component: DscprofilesetterComponent;
  let fixture: ComponentFixture<DscprofilesetterComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ DscprofilesetterComponent ],
      imports: [
        RouterTestingModule,
        HttpClientTestingModule,
        NoopAnimationsModule,
        MaterialdesignModule,
        PrimengModule,
        FormsModule,
      ],
      providers: []
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DscprofilesetterComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
