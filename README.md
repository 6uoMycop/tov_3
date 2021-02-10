<h1 class="code-line" data-line-start=0 data-line-end=1 ><a id="______0"></a>Программное средство, выполняющее полиморфизм этапа сборки.</h1>
<h3 class="code-line" data-line-start=2 data-line-end=3 ><a id="__2"></a>Алгоритм работы:</h3>
<p class="has-line-data" data-line-start="3" data-line-end="4"><em>Перед сборкой</em></p>
<ol>
<li class="has-line-data" data-line-start="4" data-line-end="5">Создать копию исходного файла для восстановления</li>
</ol>
<pre><code>echo F|xcopy SourceProg.cpp tmp.cpp
</code></pre>
<ol start="2">
<li class="has-line-data" data-line-start="9" data-line-end="10">Выполнить обфускацию на уровне исходного кода начиная с заданной строки</li>
</ol>
<pre><code>python obfuscator.py SourceProg.cpp 237
</code></pre>
<p class="has-line-data" data-line-start="14" data-line-end="16"><strong>СБОРКА</strong><br>
<em>После сборки</em></p>
<ol start="3">
<li class="has-line-data" data-line-start="16" data-line-end="17">Выполнить зашифрование участка исполняемого файла между заданными адресами на случайном ключе</li>
</ol>
<pre><code>encrypter.exe $(TargetPath) 33FA0 34EDA
</code></pre>
<ol start="4">
<li class="has-line-data" data-line-start="21" data-line-end="22">Восстановить исходный файл из копии</li>
</ol>
<pre><code>del SourceProg.cpp                 
echo F|xcopy tmp.cpp SourceProg.cpp
del tmp.cpp                        
</code></pre>
<p class="has-line-data" data-line-start="28" data-line-end="29"><em>Выполнение</em></p>
<ol start="5">
<li class="has-line-data" data-line-start="29" data-line-end="30">Установить права чтения, записи и исполнения для зашифрованного участка кода, в котором содержится полезная нагрузка</li>
</ol>
<pre><code>void getRWX(void* startAddr)
{
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(
        (void*)startAddr,
        &amp;mbi,
        sizeof(mbi));
    VirtualProtect(
        mbi.BaseAddress,
        mbi.RegionSize,
        PAGE_EXECUTE_READWRITE,
        &amp;mbi.Protect);
}
</code></pre>
<ol start="6">
<li class="has-line-data" data-line-start="46" data-line-end="47">Расшифровать участок кода в процессе выполнения</li>
<li class="has-line-data" data-line-start="47" data-line-end="49">Выполнить расшифрованную полезную нагрузку программы</li>
</ol>
<p class="has-line-data" data-line-start="49" data-line-end="50">Для обеспечения полиморфизма используется два модуля: обфускатор (<a href="http://obfuscator.py">obfuscator.py</a>) и шифровальщик  (encrypter.exe). Затем зашифрованный код расшифровывается в процессе выполнения программы.</p>
<p class="has-line-data" data-line-start="51" data-line-end="52">Обфускатор производит модификацию исходного кода программы, состоящую в добавлении ассемблерных инструкций, не изменяющих общую логику. С некоторой вероятностью могут быть добавлены следующие инструкции:</p>
<pre><code>NOP
PUSH, POP со случайным регистром общего назначения
INC, DEC со случайным регистром общего назначения
</code></pre>
<p class="has-line-data" data-line-start="57" data-line-end="60">или их комбинации. Обфусцируются функции, которые не могут быть зашифрованы: функция main() и функции, необходимые для расшифрования.<br>
Шифровальщик выполняет операцию XOR с однобайтовым случайным ключом для участка исполняемого файла, в котором содержится код полезной нагрузки.<br>
В процессе выполнения вычисляется ключ и производится обратное преобразование зашифрованного участка кода. Код любой функции начинается с операции PUSH EBP, код операции 0x55. Таким образом, ключ можно получить так: key=b_0⨁0x55, где b_0 – начальный байт зашифрованной функции.</p>
